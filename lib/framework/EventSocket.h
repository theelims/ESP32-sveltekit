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
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <PsychicHttp.h>
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
  EventSocket(PsychicHttpServer *server, SecurityManager *_securityManager, AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_AUTHENTICATED);

  void begin();

  void registerEvent(String event);

  void onEvent(String event, EventCallback callback);

  void onSubscribe(String event, SubscribeCallback callback);

  void emitEvent(String event, JsonObject &jsonObject, const char *originId = "", bool onlyToSameOrigin = false);
  // if onlyToSameOrigin == true, the message will be sent to the originId only, otherwise it will be broadcasted to all clients except the originId

private:
  PsychicHttpServer *_server;
  PsychicWebSocketHandler _socket;
  SecurityManager *_securityManager;
  AuthenticationPredicate _authenticationPredicate;

  std::vector<String> events;
  std::map<String, std::list<int>> client_subscriptions;
  std::map<String, std::list<EventCallback>> event_callbacks;
  std::map<String, std::list<SubscribeCallback>> subscribe_callbacks;
  void handleEventCallbacks(String event, JsonObject &jsonObject, int originId);
  void handleSubscribeCallbacks(String event, const String &originId);

  bool isEventValid(String event);

  void onWSOpen(PsychicWebSocketClient *client);
  void onWSClose(PsychicWebSocketClient *client);
  esp_err_t onFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame);
};

#endif
