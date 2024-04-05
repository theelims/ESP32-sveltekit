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

#include <PsychicHttp.h>
#include <SecurityManager.h>
#include <StatefulService.h>
#include <list>

#define WEB_SOCKET_SERVICE_PATH "/ws"
#define EVENT_NOTIFICATION_SERVICE_PATH "/events"

typedef std::function<void(JsonObject &root)> EventCallback;

enum pushEvent
{
    PUSHERROR,
    PUSHWARNING,
    PUSHINFO,
    PUSHSUCCESS
};

class Socket
{
  public:
    Socket(PsychicHttpServer *server, SecurityManager *_securityManager,
           AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_AUTHENTICATED);

    void begin();

    void on(String event, EventCallback callback);

    void emit(JsonObject root, String event, size_t dataSize);

    void emit(String message, String event);

    void pushNotification(String message, pushEvent event);

    void broadcast(String message);

  private:
    PsychicHttpServer *_server;
    PsychicWebSocketHandler _socket;
    SecurityManager *_securityManager;
    AuthenticationPredicate _authenticationPredicate;
    PsychicEventSource _eventSource;
    std::map<String, std::list<int>> client_subscriptions;
    std::map<String, std::list<EventCallback>> event_callbacks;
    void handleCallbacks(String event, JsonObject &jsonObject);

    size_t _bufferSize;
    void onWSOpen(PsychicWebSocketClient *client);
    void onWSClose(PsychicWebSocketClient *client);
    esp_err_t onFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame);
};

#endif
