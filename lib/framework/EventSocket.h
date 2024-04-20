#ifndef Socket_h
#define Socket_h

#include <PsychicHttp.h>
#include <SecurityManager.h>
#include <StatefulService.h>
#include <list>
#include <map>
#include <vector>

#define EVENT_SERVICE_PATH "/ws/events"

typedef std::function<void(JsonObject &root, int originId)> EventCallback;
typedef std::function<void(const String &originId, bool sync)> SubscribeCallback;

enum pushEvent
{
  PUSHERROR,
  PUSHWARNING,
  PUSHINFO,
  PUSHSUCCESS
};

class EventSocket
{
public:
  EventSocket(PsychicHttpServer *server, SecurityManager *_securityManager, AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_AUTHENTICATED);

  void begin();

  void registerEvent(String event);

  void onEvent(String event, EventCallback callback);

  void onSubscribe(String event, SubscribeCallback callback);

  void emit(String event, String payload);

  void emit(const char *event, const char *payload);

  void emit(const char *event, const char *payload, const char *originId, bool onlyToSameOrigin = false);
  // if onlyToSameOrigin == true, the message will be sent to the originId only, otherwise it will be broadcasted to all clients except the originId

  void pushNotification(String message, pushEvent event);

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

  size_t _bufferSize;
  void onWSOpen(PsychicWebSocketClient *client);
  void onWSClose(PsychicWebSocketClient *client);
  esp_err_t onFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame);
};

#endif
