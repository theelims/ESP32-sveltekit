#include <NotificationService.h>

// array translating pushType into strings
const char *pushTypeStrings[] = {"error", "warning", "info", "success"};

NotificationService::NotificationService(EventSocket *eventSocket) : _eventSocket(eventSocket)
{
}

void NotificationService::begin()
{
    _eventSocket->registerEvent(NOTIFICATION_EVENT);
}

void NotificationService::pushNotification(String message, pushType event)
{
    JsonDocument doc;
    doc["type"] = pushTypeStrings[event];
    doc["message"] = message;
    JsonObject jsonObject = doc.as<JsonObject>();
    _eventSocket->emitEvent(NOTIFICATION_EVENT, jsonObject);
}
