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
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <StatefulService.h>
#include <ESPAsyncWebServer.h>
#include <SecurityManager.h>
#include "freertos/timers.h"

#define WEB_SOCKET_CLIENT_ID_MSG_SIZE 128

#define WEB_SOCKET_ORIGIN "wsserver"
#define WEB_SOCKET_ORIGIN_CLIENT_ID_PREFIX "wsserver:"

#define WEB_SOCKET_CLIENT_CLEANUP_INTERVAL 1000

template <class T>
class WebSocketServerConnector
{
protected:
    StatefulService<T> *_statefulService;
    AsyncWebServer *_server;
    AsyncWebSocket _webSocket;
    size_t _bufferSize;

    WebSocketServerConnector(StatefulService<T> *statefulService,
                             AsyncWebServer *server,
                             const char *webSocketPath,
                             SecurityManager *securityManager,
                             AuthenticationPredicate authenticationPredicate,
                             size_t bufferSize) : _statefulService(statefulService), _server(server), _webSocket(webSocketPath), _bufferSize(bufferSize)
    {
        _webSocket.setFilter(securityManager->filterRequest(authenticationPredicate));
        _webSocket.onEvent(std::bind(&WebSocketServerConnector::onWSEvent,
                                     this,
                                     std::placeholders::_1,
                                     std::placeholders::_2,
                                     std::placeholders::_3,
                                     std::placeholders::_4,
                                     std::placeholders::_5,
                                     std::placeholders::_6));
        _server->addHandler(&_webSocket);
        _server->on(webSocketPath, HTTP_GET, std::bind(&WebSocketServerConnector::forbidden, this, std::placeholders::_1));
    }

    WebSocketServerConnector(StatefulService<T> *statefulService,
                             AsyncWebServer *server,
                             const char *webSocketPath,
                             size_t bufferSize) : _statefulService(statefulService), _server(server), _webSocket(webSocketPath), _bufferSize(bufferSize)
    {
        _webSocket.onEvent(std::bind(&WebSocketServerConnector::onWSEvent,
                                     this,
                                     std::placeholders::_1,
                                     std::placeholders::_2,
                                     std::placeholders::_3,
                                     std::placeholders::_4,
                                     std::placeholders::_5,
                                     std::placeholders::_6));
        _server->addHandler(&_webSocket);
    }

    virtual void onWSEvent(AsyncWebSocket *server,
                           AsyncWebSocketClient *client,
                           AwsEventType type,
                           void *arg,
                           uint8_t *data,
                           size_t len) = 0;

    String clientId(AsyncWebSocketClient *client)
    {
        return WEB_SOCKET_ORIGIN_CLIENT_ID_PREFIX + String(client->id());
    }

private:
    void forbidden(AsyncWebServerRequest *request)
    {
        request->send(403);
    }
};

template <class T>
class WebSocketTx : virtual public WebSocketServerConnector<T>
{
public:
    WebSocketTx(JsonStateReader<T> stateReader,
                StatefulService<T> *statefulService,
                AsyncWebServer *server,
                const char *webSocketPath,
                SecurityManager *securityManager,
                AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_ADMIN,
                size_t bufferSize = DEFAULT_BUFFER_SIZE) : WebSocketServerConnector<T>(statefulService,
                                                                                       server,
                                                                                       webSocketPath,
                                                                                       securityManager,
                                                                                       authenticationPredicate,
                                                                                       bufferSize),
                                                           _stateReader(stateReader)
    {
        WebSocketServerConnector<T>::_statefulService->addUpdateHandler(
            [&](const String &originId)
            { transmitData(nullptr, originId); },
            false);
    }

    WebSocketTx(JsonStateReader<T> stateReader,
                StatefulService<T> *statefulService,
                AsyncWebServer *server,
                const char *webSocketPath,
                size_t bufferSize = DEFAULT_BUFFER_SIZE) : WebSocketServerConnector<T>(statefulService, server, webSocketPath, bufferSize), _stateReader(stateReader)
    {
        WebSocketServerConnector<T>::_statefulService->addUpdateHandler(
            [&](const String &originId)
            { transmitData(nullptr, originId); },
            false);
    }

protected:
    virtual void onWSEvent(AsyncWebSocket *server,
                           AsyncWebSocketClient *client,
                           AwsEventType type,
                           void *arg,
                           uint8_t *data,
                           size_t len)
    {
        if (type == WS_EVT_CONNECT)
        {
            // when a client connects, we transmit it's id and the current payload
            transmitId(client);
            transmitData(client, WEB_SOCKET_ORIGIN);
        }
    }

private:
    JsonStateReader<T> _stateReader;

    void transmitId(AsyncWebSocketClient *client)
    {
        DynamicJsonDocument jsonDocument = DynamicJsonDocument(WEB_SOCKET_CLIENT_ID_MSG_SIZE);
        JsonObject root = jsonDocument.to<JsonObject>();
        root["type"] = "id";
        root["id"] = WebSocketServerConnector<T>::clientId(client);
        size_t len = measureJson(jsonDocument);
        AsyncWebSocketMessageBuffer *buffer = WebSocketServerConnector<T>::_webSocket.makeBuffer(len);
        if (buffer)
        {
            serializeJson(jsonDocument, (char *)buffer->get(), len + 1);
            client->text(buffer);
        }
    }

    /**
     * Broadcasts the payload to the destination, if provided. Otherwise broadcasts to all clients except the origin, if
     * specified.
     *
     * Original implementation sent clients their own IDs so they could ignore updates they initiated. This approach
     * simplifies the client and the server implementation but may not be sufficent for all use-cases.
     */
    void transmitData(AsyncWebSocketClient *client, const String &originId)
    {
        DynamicJsonDocument jsonDocument = DynamicJsonDocument(WebSocketServerConnector<T>::_bufferSize);
        JsonObject root = jsonDocument.to<JsonObject>();
        root["type"] = "payload";
        root["origin_id"] = originId;
        JsonObject payload = root.createNestedObject("payload");
        WebSocketServerConnector<T>::_statefulService->read(payload, _stateReader);

        size_t len = measureJson(jsonDocument);
        AsyncWebSocketMessageBuffer *buffer = WebSocketServerConnector<T>::_webSocket.makeBuffer(len);
        if (buffer)
        {
            serializeJson(jsonDocument, (char *)buffer->get(), len + 1);
            if (client)
            {
                client->text(buffer);
            }
            else
            {
                WebSocketServerConnector<T>::_webSocket.textAll(buffer);
            }
        }
    }
};

template <class T>
class WebSocketRx : virtual public WebSocketServerConnector<T>
{
public:
    WebSocketRx(JsonStateUpdater<T> stateUpdater,
                StatefulService<T> *statefulService,
                AsyncWebServer *server,
                const char *webSocketPath,
                SecurityManager *securityManager,
                AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_ADMIN,
                size_t bufferSize = DEFAULT_BUFFER_SIZE) : WebSocketServerConnector<T>(statefulService,
                                                                                       server,
                                                                                       webSocketPath,
                                                                                       securityManager,
                                                                                       authenticationPredicate,
                                                                                       bufferSize),
                                                           _stateUpdater(stateUpdater)
    {
    }

    WebSocketRx(JsonStateUpdater<T> stateUpdater,
                StatefulService<T> *statefulService,
                AsyncWebServer *server,
                const char *webSocketPath,
                size_t bufferSize = DEFAULT_BUFFER_SIZE) : WebSocketServerConnector<T>(statefulService, server, webSocketPath, bufferSize), _stateUpdater(stateUpdater)
    {
    }

protected:
    virtual void onWSEvent(AsyncWebSocket *server,
                           AsyncWebSocketClient *client,
                           AwsEventType type,
                           void *arg,
                           uint8_t *data,
                           size_t len)
    {
        if (type == WS_EVT_DATA)
        {
            AwsFrameInfo *info = (AwsFrameInfo *)arg;
            if (info->final && info->index == 0 && info->len == len)
            {
                if (info->opcode == WS_TEXT)
                {
                    DynamicJsonDocument jsonDocument = DynamicJsonDocument(WebSocketServerConnector<T>::_bufferSize);
                    DeserializationError error = deserializeJson(jsonDocument, (char *)data);
                    if (!error && jsonDocument.is<JsonObject>())
                    {
                        JsonObject jsonObject = jsonDocument.as<JsonObject>();
                        WebSocketServerConnector<T>::_statefulService->update(
                            jsonObject, _stateUpdater, WebSocketServerConnector<T>::clientId(client));
                    }
                }
            }
        }
    }

private:
    JsonStateUpdater<T> _stateUpdater;
};

template <class T>
class WebSocketServer : public WebSocketTx<T>, public WebSocketRx<T>
{
public:
    WebSocketServer(JsonStateReader<T> stateReader,
                    JsonStateUpdater<T> stateUpdater,
                    StatefulService<T> *statefulService,
                    AsyncWebServer *server,
                    const char *webSocketPath,
                    SecurityManager *securityManager,
                    AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_ADMIN,
                    size_t bufferSize = DEFAULT_BUFFER_SIZE) : WebSocketServerConnector<T>(statefulService,
                                                                                           server,
                                                                                           webSocketPath,
                                                                                           securityManager,
                                                                                           authenticationPredicate,
                                                                                           bufferSize),
                                                               WebSocketTx<T>(stateReader,
                                                                              statefulService,
                                                                              server,
                                                                              webSocketPath,
                                                                              securityManager,
                                                                              authenticationPredicate,
                                                                              bufferSize),
                                                               WebSocketRx<T>(stateUpdater,
                                                                              statefulService,
                                                                              server,
                                                                              webSocketPath,
                                                                              securityManager,
                                                                              authenticationPredicate,
                                                                              bufferSize)
    {
        // Create the FreeRTOS timer with a period of 1000ms (1 second)
        timerHandle = xTimerCreate("CleanupTimer", pdMS_TO_TICKS(WEB_SOCKET_CLIENT_CLEANUP_INTERVAL), pdTRUE, this, timerCallbackStatic);

        // Start the timer (it will call the cleanupFunction every 1000ms)
        xTimerStart(timerHandle, portMAX_DELAY);
    }

    WebSocketServer(JsonStateReader<T> stateReader,
                    JsonStateUpdater<T> stateUpdater,
                    StatefulService<T> *statefulService,
                    AsyncWebServer *server,
                    const char *webSocketPath,
                    size_t bufferSize = DEFAULT_BUFFER_SIZE) : WebSocketServerConnector<T>(statefulService, server, webSocketPath, bufferSize),
                                                               WebSocketTx<T>(stateReader, statefulService, server, webSocketPath, bufferSize),
                                                               WebSocketRx<T>(stateUpdater, statefulService, server, webSocketPath, bufferSize)
    {
        // Create the FreeRTOS timer with a period of 1000ms (1 second)
        timerHandle = xTimerCreate("CleanupTimer", pdMS_TO_TICKS(WEB_SOCKET_CLIENT_CLEANUP_INTERVAL), pdTRUE, this, timerCallbackStatic);

        // Start the timer (it will call the cleanupFunction every 1000ms)
        xTimerStart(timerHandle, portMAX_DELAY);
    }

protected:
    void cleanupClientsWrapper()
    {
        WebSocketServerConnector<T>::_webSocket.cleanupClients();
        // ESP_LOGV("WebSocket Server", "Cleanup WS Clients");
    }

    static void timerCallbackStatic(TimerHandle_t xTimer)
    {
        // Get the instance of the class from the timer's user data
        WebSocketServer *instance = static_cast<WebSocketServer *>(pvTimerGetTimerID(xTimer));
        // Call the member function using the class instance
        if (instance)
        {
            instance->cleanupClientsWrapper();
        }
    }

    void onWSEvent(AsyncWebSocket *server,
                   AsyncWebSocketClient *client,
                   AwsEventType type,
                   void *arg,
                   uint8_t *data,
                   size_t len)
    {
        WebSocketRx<T>::onWSEvent(server, client, type, arg, data, len);
        WebSocketTx<T>::onWSEvent(server, client, type, arg, data, len);
    }

private:
    AsyncWebSocket *_webSocket;
    TimerHandle_t timerHandle;
};

#endif
