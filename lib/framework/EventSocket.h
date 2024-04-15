#ifndef Socket_h
#define Socket_h

#include <PsychicHttp.h>
#include <SecurityManager.h>
#include <StatefulService.h>
#include <list>

#define EVENT_SERVICE_PATH "/events"
#define WS_EVENT_SERVICE_PATH "/ws"

typedef std::function<void(JsonObject &root)> EventCallback;

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
    EventSocket(PsychicHttpServer *server, SecurityManager *_securityManager,
                AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_AUTHENTICATED);

    void begin();

    void on(String event, EventCallback callback);

    void emit(String event, String payload);

    void emit(const char *event, const char *payload);

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
