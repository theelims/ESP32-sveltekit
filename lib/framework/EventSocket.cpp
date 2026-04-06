#include <EventSocket.h>

SemaphoreHandle_t clientSubscriptionsMutex = xSemaphoreCreateMutex();

EventSocket::EventSocket(AsyncWebServer *server,
                         SecurityManager *securityManager,
                         AuthenticationPredicate authenticationPredicate) : _server(server),
                                                                            _socket(EVENT_SERVICE_PATH),
                                                                            _securityManager(securityManager),
                                                                            _authenticationPredicate(authenticationPredicate)
{
}

void EventSocket::begin()
{
    _socket.setFilter(_securityManager->filterRequest(_authenticationPredicate));
    _socket.onEvent(std::bind(&EventSocket::onWSEvent, this,
                              std::placeholders::_1, std::placeholders::_2,
                              std::placeholders::_3, std::placeholders::_4,
                              std::placeholders::_5, std::placeholders::_6));
    _server->addHandler(&_socket);

    ESP_LOGV(SVK_TAG, "Registered event socket endpoint: %s", EVENT_SERVICE_PATH);
}

void EventSocket::registerEvent(String event)
{
    if (!isEventValid(event))
    {
        ESP_LOGD(SVK_TAG, "Registering event: %s", event.c_str());
        events.push_back(event);
    }
    else
    {
        ESP_LOGW(SVK_TAG, "Event already registered: %s", event.c_str());
    }
}

void EventSocket::onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                             AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        onWSOpen(client);
        break;
    case WS_EVT_DISCONNECT:
        onWSClose(client);
        break;
    case WS_EVT_DATA:
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len)
        {
            onFrame(client, info, data, len);
        }
        break;
    }
    default:
        break;
    }
}

void EventSocket::onWSOpen(AsyncWebSocketClient *client)
{
    ESP_LOGI(SVK_TAG, "ws[%s][%u] connect", client->remoteIP().toString().c_str(), client->id());
}

void EventSocket::onWSClose(AsyncWebSocketClient *client)
{
    int clientId = (int)client->id();
    xSemaphoreTake(clientSubscriptionsMutex, portMAX_DELAY);
    for (auto &event_subscriptions : client_subscriptions)
    {
        event_subscriptions.second.remove(clientId);
    }
    xSemaphoreGive(clientSubscriptionsMutex);
    ESP_LOGI(SVK_TAG, "ws[%s][%u] disconnect", client->remoteIP().toString().c_str(), client->id());
}

void EventSocket::onFrame(AsyncWebSocketClient *client, AwsFrameInfo *info, uint8_t *data, size_t len)
{
    int clientId = (int)client->id();
    ESP_LOGV(SVK_TAG, "ws[%s][%u] opcode[%d]", client->remoteIP().toString().c_str(), client->id(), info->opcode);

    JsonDocument doc;
#if FT_ENABLED(EVENT_USE_JSON)
    if (info->opcode == WS_TEXT)
    {
        ESP_LOGV(SVK_TAG, "ws[%s][%u] request: %s", client->remoteIP().toString().c_str(),
                 client->id(), (char *)data);

        DeserializationError error = deserializeJson(doc, (char *)data, len);
#else
    if (info->opcode == WS_BINARY)
    {
        ESP_LOGV(SVK_TAG, "ws[%s][%u] request: %s", client->remoteIP().toString().c_str(),
                 client->id(), (char *)data);

        DeserializationError error = deserializeMsgPack(doc, (char *)data, len);
#endif

        if (!error && doc.is<JsonObject>())
        {
            String event = doc["event"];
            if (event == "subscribe")
            {
                if (isEventValid(doc["data"].as<String>()))
                {
                    client_subscriptions[doc["data"]].push_back(clientId);
                    handleSubscribeCallbacks(doc["data"], String(clientId));
                }
                else
                {
                    ESP_LOGW(SVK_TAG, "Client tried to subscribe to unregistered event: %s", doc["data"].as<String>().c_str());
                }
            }
            else if (event == "unsubscribe")
            {
                client_subscriptions[doc["data"]].remove(clientId);
            }
            else
            {
                JsonObject jsonObject = doc["data"].as<JsonObject>();
                handleEventCallbacks(event, jsonObject, clientId);
            }
            return;
        }
        ESP_LOGW(SVK_TAG, "Error[%d] parsing JSON: %s", error, (char *)data);
    }
}

void EventSocket::emitEvent(String event, JsonObject &jsonObject, const char *originId, bool onlyToSameOrigin)
{
    if (!isEventValid(String(event)))
    {
        ESP_LOGW(SVK_TAG, "Method tried to emit unregistered event: %s", event);
        return;
    }

    int originSubscriptionId = originId[0] ? atoi(originId) : -1;

    std::vector<int> targetClients;
    JsonDocument doc;
    doc["event"] = event;
    doc["data"] = jsonObject;

#if FT_ENABLED(EVENT_USE_JSON)
    size_t len = measureJson(doc);
#else
    size_t len = measureMsgPack(doc);
#endif

    char *output = new char[len + 1];

#if FT_ENABLED(EVENT_USE_JSON)
    serializeJson(doc, output, len + 1);
#else
    serializeMsgPack(doc, output, len);
#endif

    output[len] = '\0';

    xSemaphoreTake(clientSubscriptionsMutex, portMAX_DELAY);
    auto &subscriptions = client_subscriptions[event];
    if (subscriptions.empty())
    {
        xSemaphoreGive(clientSubscriptionsMutex);
        delete[] output;
        return;
    }

    if (onlyToSameOrigin && originSubscriptionId > 0)
    {
        targetClients.push_back(originSubscriptionId);
    }
    else
    {
        for (int subscription : subscriptions)
        {
            if (subscription != originSubscriptionId)
                targetClients.push_back(subscription);
        }
    }
    xSemaphoreGive(clientSubscriptionsMutex);

    std::vector<int> toRemove;
    for (int subscription : targetClients)
    {
        auto *client = _socket.client((uint32_t)subscription);
        if (!client)
        {
            toRemove.push_back(subscription);
            continue;
        }
        ESP_LOGV(SVK_TAG, "Emitting event: %s to %s[%u], Message[%d]: %s", event, client->remoteIP().toString().c_str(), client->id(), len, output);
#if FT_ENABLED(EVENT_USE_JSON)
        client->text(output, len);
#else
        client->binary(output, len);
#endif
    }

    if (!toRemove.empty())
    {
        xSemaphoreTake(clientSubscriptionsMutex, portMAX_DELAY);
        for (int id : toRemove)
            client_subscriptions[event].remove(id);
        xSemaphoreGive(clientSubscriptionsMutex);
    }

    delete[] output;
}

void EventSocket::handleEventCallbacks(String event, JsonObject &jsonObject, int originId)
{
    for (auto &callback : event_callbacks[event])
    {
        callback(jsonObject, originId);
    }
}

void EventSocket::handleSubscribeCallbacks(String event, const String &originId)
{
    for (auto &callback : subscribe_callbacks[event])
    {
        callback(originId);
    }
}

void EventSocket::onEvent(String event, EventCallback callback)
{
    if (!isEventValid(event))
    {
        ESP_LOGW(SVK_TAG, "Method tried to register unregistered event: %s", event.c_str());
        return;
    }
    event_callbacks[event].push_back(callback);
}

void EventSocket::onSubscribe(String event, SubscribeCallback callback)
{
    if (!isEventValid(event))
    {
        ESP_LOGW(SVK_TAG, "Method tried to subscribe to unregistered event: %s", event.c_str());
        return;
    }
    subscribe_callbacks[event].push_back(callback);
    ESP_LOGI(SVK_TAG, "onSubscribe for event: %s", event.c_str());
}

bool EventSocket::isEventValid(String event)
{
    return std::find(events.begin(), events.end(), event) != events.end();
}

unsigned int EventSocket::getConnectedClients()
{
    return (unsigned int)_socket.count();
}
