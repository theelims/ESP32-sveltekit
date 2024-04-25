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
    // convert into JsonVariant

    String output;
    serializeJson(doc, output);
    _eventSocket->emit(NOTIFICATION_EVENT, output.c_str());
}
