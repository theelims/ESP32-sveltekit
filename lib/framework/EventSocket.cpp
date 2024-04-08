/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <EventSocket.h>

EventSocket::EventSocket(PsychicHttpServer *server, SecurityManager *securityManager,
                         AuthenticationPredicate authenticationPredicate)
    : _server(server), _securityManager(securityManager), _authenticationPredicate(authenticationPredicate),
      _bufferSize(1024)
{
}

void EventSocket::begin()
{
    _socket.setFilter(_securityManager->filterRequest(_authenticationPredicate));
    _socket.onOpen((std::bind(&EventSocket::onWSOpen, this, std::placeholders::_1)));
    _socket.onClose(std::bind(&EventSocket::onWSClose, this, std::placeholders::_1));
    _socket.onFrame(std::bind(&EventSocket::onFrame, this, std::placeholders::_1, std::placeholders::_2));
    _server->on(EVENT_SERVICE_PATH, &_eventSource);
    _server->on(WS_EVENT_SERVICE_PATH, &_socket);

    ESP_LOGV("Socket", "Registered socket Source endpoint: %s", EVENT_SERVICE_PATH);
}

void EventSocket::onWSOpen(PsychicWebSocketClient *client)
{
    ESP_LOGI("WebSocketServer", "ws[%s][%u] connect", client->remoteIP().toString(), client->socket());
}

void EventSocket::onWSClose(PsychicWebSocketClient *client)
{
    for (auto &event_subscriptions : client_subscriptions)
    {
        event_subscriptions.second.remove(client->socket());
    }
    ESP_LOGI("WebSocketServer", "ws[%s][%u] disconnect", client->remoteIP().toString(), client->socket());
}

esp_err_t EventSocket::onFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame)
{
    ESP_LOGV("WebSocketServer", "ws[%s][%u] opcode[%d]", request->client()->remoteIP().toString(),
             request->client()->socket(), frame->type);

    if (frame->type == HTTPD_WS_TYPE_TEXT)
    {
        ESP_LOGV("WebSocketServer", "ws[%s][%u] request: %s", request->client()->remoteIP().toString(),
                 request->client()->socket(), (char *)frame->payload);

        DynamicJsonDocument doc = DynamicJsonDocument(_bufferSize);
        DeserializationError error = deserializeJson(doc, (char *)frame->payload, frame->len);

        if (!error && doc.is<JsonObject>())
        {
            String event = doc["event"];
            if (event == "subscribe")
            {
                client_subscriptions[doc["data"]].push_back(request->client()->socket());
            }
            else if (event == "unsubscribe")
            {
                client_subscriptions[doc["data"]].remove(request->client()->socket());
            }
            else
            {
                JsonObject jsonObject = doc["data"].as<JsonObject>();
                handleCallbacks(event, jsonObject);
            }
            return ESP_OK;
        }
    }
    return ESP_OK;
}

void EventSocket::emit(JsonObject root, String event, size_t dataSize)
{
    if (client_subscriptions[event].size() == 0)
        return;
    DynamicJsonDocument doc(dataSize + 100);
    doc["event"] = event;
    doc["data"] = root;
    char buffer[dataSize + 100];
    if (_eventSource.count() > 0)
    {
        serializeJson(root, buffer, sizeof(buffer));
        _eventSource.send(buffer, event.c_str(), millis());
    }
    serializeJson(doc, buffer, sizeof(buffer));

    for (int subscription : client_subscriptions[event])
    {
        auto *client = _socket.getClient(subscription);
        if (!client)
        {
            client_subscriptions[event].remove(subscription);
            continue;
        }
        ESP_LOGV("WebSocketServer", "Emitting event: %s to %s, Message: %s", event.c_str(),
                 client->remoteIP().toString(), buffer);
        client->sendMessage(buffer);
    }
}

void EventSocket::emit(String message, String event)
{
    if (_eventSource.count() > 0)
    {
        _eventSource.send(message.c_str(), event.c_str(), millis());
    }
    if (client_subscriptions[event].size() == 0)
        return;
    size_t dataSize = message.length() + 1;
    DynamicJsonDocument doc(dataSize + 100);
    doc["event"] = event;
    doc["data"] = message;
    char buffer[dataSize + 100];
    serializeJson(doc, buffer, sizeof(buffer));
    for (int subscription : client_subscriptions[event])
    {
        auto *client = _socket.getClient(subscription);
        if (!client)
        {
            client_subscriptions[event].remove(subscription);
            continue;
        }
        ESP_LOGV("WebSocketServer", "Emitting event: %s to %s, Message: %s", event.c_str(),
                 client->remoteIP().toString(), buffer);
        client->sendMessage(buffer);
    }
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
    emit(message, eventType);
}

void EventSocket::handleCallbacks(String event, JsonObject &jsonObject)
{
    for (auto &callback : event_callbacks[event])
    {
        callback(jsonObject);
    }
}

void EventSocket::on(String event, EventCallback callback)
{
    event_callbacks[event].push_back(callback);
    log_d("Socket::on");
}

void EventSocket::broadcast(String message)
{
    _socket.sendAll(message.c_str());
}
