#ifndef WebSocketServer_h
#define WebSocketServer_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <StatefulService.h>
#include <ESPAsyncWebServer.h>
#include <SecurityManager.h>

#define WEB_SOCKET_ORIGIN "wsserver"
#define WEB_SOCKET_ORIGIN_CLIENT_ID_PREFIX "wsserver:"

template <class T>
class WebSocketServer
{
public:
    WebSocketServer(JsonStateReader<T> stateReader,
                    JsonStateUpdater<T> stateUpdater,
                    StatefulService<T> *statefulService,
                    AsyncWebServer *server,
                    const char *webSocketPath,
                    SecurityManager *securityManager,
                    AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_ADMIN) : _stateReader(stateReader),
                                                                                                            _stateUpdater(stateUpdater),
                                                                                                            _statefulService(statefulService),
                                                                                                            _server(server),
                                                                                                            _webSocketPath(webSocketPath),
                                                                                                            _webSocket(webSocketPath),
                                                                                                            _authenticationPredicate(authenticationPredicate),
                                                                                                            _securityManager(securityManager)
    {
        _statefulService->addUpdateHandler(
            [&](const String &originId)
            { transmitData(nullptr, originId); },
            false);
    }

    void begin()
    {
        _webSocket.setFilter(_securityManager->filterRequest(_authenticationPredicate));
        _webSocket.onEvent(std::bind(&WebSocketServer::onWSEvent,
                                     this,
                                     std::placeholders::_1,
                                     std::placeholders::_2,
                                     std::placeholders::_3,
                                     std::placeholders::_4,
                                     std::placeholders::_5,
                                     std::placeholders::_6));
        _server->addHandler(&_webSocket);

        ESP_LOGV(SVK_TAG, "Registered WebSocket handler: %s", _webSocketPath.c_str());
    }

    void onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
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
                onWSFrame(client, info, data, len);
            }
            break;
        }
        default:
            break;
        }
    }

    void onWSOpen(AsyncWebSocketClient *client)
    {
        transmitId(client);
        transmitData(client, WEB_SOCKET_ORIGIN);
        ESP_LOGI(SVK_TAG, "ws[%s][%u] connect", client->remoteIP().toString().c_str(), client->id());
    }

    void onWSClose(AsyncWebSocketClient *client)
    {
        ESP_LOGI(SVK_TAG, "ws[%s][%u] disconnect", client->remoteIP().toString().c_str(), client->id());
    }

    void onWSFrame(AsyncWebSocketClient *client, AwsFrameInfo *info, uint8_t *data, size_t len)
    {
        ESP_LOGV(SVK_TAG, "ws[%s][%u] opcode[%d]", client->remoteIP().toString().c_str(), client->id(), info->opcode);

        if (info->opcode == WS_TEXT)
        {
            ESP_LOGV(SVK_TAG, "ws[%s][%u] request: %s", client->remoteIP().toString().c_str(), client->id(), (char *)data);

            JsonDocument jsonDocument;
            DeserializationError error = deserializeJson(jsonDocument, (char *)data, len);

            if (!error && jsonDocument.is<JsonObject>())
            {
                JsonObject jsonObject = jsonDocument.as<JsonObject>();
                _statefulService->update(jsonObject, _stateUpdater, clientId(client));
            }
        }
    }

    String clientId(AsyncWebSocketClient *client)
    {
        return WEB_SOCKET_ORIGIN_CLIENT_ID_PREFIX + String(client->id());
    }

private:
    JsonStateReader<T> _stateReader;
    JsonStateUpdater<T> _stateUpdater;
    StatefulService<T> *_statefulService;
    AuthenticationPredicate _authenticationPredicate;
    SecurityManager *_securityManager;
    AsyncWebServer *_server;
    String _webSocketPath;
    AsyncWebSocket _webSocket;

    void transmitId(AsyncWebSocketClient *client)
    {
        JsonDocument jsonDocument;
        JsonObject root = jsonDocument.to<JsonObject>();
        root["type"] = "id";
        root["id"] = clientId(client);

        String buffer;
        serializeJson(jsonDocument, buffer);
        client->text(buffer.c_str());
    }

    void transmitData(AsyncWebSocketClient *client, const String &originId)
    {
        JsonDocument jsonDocument;
        JsonObject root = jsonDocument.to<JsonObject>();
        String buffer;

        _statefulService->read(root, _stateReader);

        serializeJson(jsonDocument, buffer);
        if (client)
        {
            client->text(buffer.c_str());
        }
        else
        {
            _webSocket.textAll(buffer.c_str());
        }
    }
};

#endif
