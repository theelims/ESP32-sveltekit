#include <EventSocket.h>

SemaphoreHandle_t clientSubscriptionsMutex = xSemaphoreCreateMutex();

EventSocket::EventSocket(PsychicHttpServer *server,
                         SecurityManager *securityManager,
                         AuthenticationPredicate authenticationPredicate) : _server(server),
                                                                            _securityManager(securityManager),
                                                                            _authenticationPredicate(authenticationPredicate),
                                                                            _bufferSize(1024)
{
}

void EventSocket::begin()
{
    _socket.setFilter(_securityManager->filterRequest(_authenticationPredicate));
    _socket.onOpen((std::bind(&EventSocket::onWSOpen, this, std::placeholders::_1)));
    _socket.onClose(std::bind(&EventSocket::onWSClose, this, std::placeholders::_1));
    _socket.onFrame(std::bind(&EventSocket::onFrame, this, std::placeholders::_1, std::placeholders::_2));
    _server->on(EVENT_SERVICE_PATH, &_socket);

    ESP_LOGV("EventSocket", "Registered event socket endpoint: %s", EVENT_SERVICE_PATH);
}

void EventSocket::onWSOpen(PsychicWebSocketClient *client)
{
    ESP_LOGI("EventSocket", "ws[%s][%u] connect", client->remoteIP().toString(), client->socket());
}

void EventSocket::onWSClose(PsychicWebSocketClient *client)
{
    for (auto &event_subscriptions : client_subscriptions)
    {
        event_subscriptions.second.remove(client->socket());
    }
    ESP_LOGI("EventSocket", "ws[%s][%u] disconnect", client->remoteIP().toString(), client->socket());
}

esp_err_t EventSocket::onFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame)
{
    ESP_LOGV("EventSocket", "ws[%s][%u] opcode[%d]", request->client()->remoteIP().toString(),
             request->client()->socket(), frame->type);

    if (frame->type == HTTPD_WS_TYPE_TEXT)
    {
        ESP_LOGV("EventSocket", "ws[%s][%u] request: %s", request->client()->remoteIP().toString(),
                 request->client()->socket(), (char *)frame->payload);

        DynamicJsonDocument doc = DynamicJsonDocument(_bufferSize);
        DeserializationError error = deserializeJson(doc, (char *)frame->payload, frame->len);

        if (!error && doc.is<JsonObject>())
        {
            String event = doc["event"];
            if (event == "subscribe")
            {
                client_subscriptions[doc["data"]].push_back(request->client()->socket());
                handleSubscribeCallbacks(doc["data"], String(request->client()->socket()));
            }
            else if (event == "unsubscribe")
            {
                client_subscriptions[doc["data"]].remove(request->client()->socket());
            }
            else
            {
                JsonObject jsonObject = doc["data"].as<JsonObject>();
                handleEventCallbacks(event, jsonObject, request->client()->socket());
            }
            return ESP_OK;
        }
    }
    return ESP_OK;
}

void EventSocket::emit(String event, String payload)
{
    emit(event.c_str(), payload.c_str(), "");
}

void EventSocket::emit(const char *event, const char *payload)
{
    emit(event, payload, "");
}

void EventSocket::emit(const char *event, const char *payload, const char *originId, bool bounce)
{
    int originSubscriptionId = originId[0] ? atoi(originId) : -1;
    xSemaphoreTake(clientSubscriptionsMutex, portMAX_DELAY);
    auto &subscriptions = client_subscriptions[event];
    if (subscriptions.empty())
    {
        xSemaphoreGive(clientSubscriptionsMutex);
        return;
    }
    String msg = "42[\"" + String(event) + "\"," + String(payload) + "]";

    // if bounce == true, send the message back to the origin
    if (bounce && originSubscriptionId > 0)
    {
        auto *client = _socket.getClient(originSubscriptionId);
        if (client)
        {
            ESP_LOGV("EventSocket", "Emitting event: %s to %s, Message: %s", event, client->remoteIP().toString(),
                     msg.c_str());
            client->sendMessage(msg.c_str());
        }
    }
    else
    { // else send the message to all other clients

        for (int subscription : client_subscriptions[event])
        {
            if (subscription == originSubscriptionId)
                continue;
            auto *client = _socket.getClient(subscription);
            if (!client)
            {
                subscriptions.remove(subscription);
                continue;
            }
            ESP_LOGV("EventSocket", "Emitting event: %s to %s, Message: %s", event, client->remoteIP().toString(),
                     msg.c_str());
            client->sendMessage(msg.c_str());
        }
    }
    xSemaphoreGive(clientSubscriptionsMutex);
}

void EventSocket::pushNotification(String message, pushEvent event)
{
    String eventType;
    switch (event)
    {
    case (PUSHERROR):
        eventType = "errorToast";
        break;
    case (PUSHWARNING):
        eventType = "warningToast";
        break;
    case (PUSHINFO):
        eventType = "infoToast";
        break;
    case (PUSHSUCCESS):
        eventType = "successToast";
        break;
    default:
        return;
    }
    emit(eventType.c_str(), message.c_str());
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
        callback(originId, true);
    }
}

void EventSocket::onEvent(String event, EventCallback callback)
{
    event_callbacks[event].push_back(callback);
}

void EventSocket::onSubscribe(String event, SubscribeCallback callback)
{
    subscribe_callbacks[event].push_back(callback);
    ESP_LOGI("EventSocket", "onSubscribe for event: %s", event.c_str());
}

void EventSocket::broadcast(String message)
{
    _socket.sendAll(message.c_str());
}
