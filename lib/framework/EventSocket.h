#ifndef Socket_h
#define Socket_h

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

#include <ESPAsyncWebServer.h>
#include <SecurityManager.h>
#include <StatefulService.h>
#include <list>
#include <map>
#include <vector>

#define EVENT_SERVICE_PATH "/ws/events"

typedef std::function<void(JsonObject &root, int originId)> EventCallback;
typedef std::function<void(const String &originId)> SubscribeCallback;

class EventSocket
{
public:
    EventSocket(AsyncWebServer *server, SecurityManager *_securityManager, AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_AUTHENTICATED);

    void begin();

    void registerEvent(String event);

    void onEvent(String event, EventCallback callback);

    void onSubscribe(String event, SubscribeCallback callback);

    void emitEvent(String event, JsonObject &jsonObject, const char *originId = "", bool onlyToSameOrigin = false);

    bool isEventValid(String event);

    unsigned int getConnectedClients();

private:
    AsyncWebServer *_server;
    AsyncWebSocket _socket;
    SecurityManager *_securityManager;
    AuthenticationPredicate _authenticationPredicate;

    std::vector<String> events;
    std::map<String, std::list<int>> client_subscriptions;
    std::map<String, std::list<EventCallback>> event_callbacks;
    std::map<String, std::list<SubscribeCallback>> subscribe_callbacks;
    void handleEventCallbacks(String event, JsonObject &jsonObject, int originId);
    void handleSubscribeCallbacks(String event, const String &originId);

    void onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                   AwsEventType type, void *arg, uint8_t *data, size_t len);
    void onWSOpen(AsyncWebSocketClient *client);
    void onWSClose(AsyncWebSocketClient *client);
    void onFrame(AsyncWebSocketClient *client, AwsFrameInfo *info, uint8_t *data, size_t len);
};

#endif
