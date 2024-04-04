#ifndef Socket_h
#define Socket_h

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

#include <StatefulService.h>
#include <PsychicHttp.h>
#include <SecurityManager.h>
#include <list>

#define WEB_SOCKET_CLIENT_ID_MSG_SIZE 128

#define WEB_SOCKET_ORIGIN "wsserver"
#define WEB_SOCKET_ORIGIN_CLIENT_ID_PREFIX "wsserver:"

#define WEB_SOCKET_SERVICE_PATH "/ws"

#define SOCKET_CONNECT "connect";
#define SOCKET_DISCONNECT "disconnect";
#define SOCKET_ERROR "error";

typedef std::function<void(JsonObject &root)> EventCallback;

class Socket
{
private:
    PsychicHttpServer *_server;
    PsychicWebSocketHandler _socket;
    std::map<String, std::list<int>> client_subscriptions;
    std::map<String, std::list<EventCallback>> event_callbacks;
    void handleCallbacks(String event, JsonObject &jsonObject);

    size_t _bufferSize;
    void onWSOpen(PsychicWebSocketClient *client);
    void onWSClose(PsychicWebSocketClient *client);
    esp_err_t onFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame);

public:
    Socket(PsychicHttpServer *server);

    void begin();

    void on(String event, EventCallback callback);
    
    void emit(JsonObject root, String event, size_t dataSize);

    void emit(String message, String event);

    void broadcast(String message);
};;

#endif
