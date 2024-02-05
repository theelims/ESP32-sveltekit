#pragma once

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <NotificationEvents.h>

NotificationEvents::NotificationEvents(PsychicHttpServer *server) : _server(server)
{
}

void NotificationEvents::begin()
{
    _eventSource.onOpen([&](PsychicEventSourceClient *client) { // client->send("hello", NULL, millis(), 1000);
        Serial.printf("New client connected to Event Source: #%u connected from %s\n", client->socket(), client->remoteIP().toString());
    });
    _eventSource.onClose([&](PsychicEventSourceClient *client) { // client->send("hello", NULL, millis(), 1000);
        Serial.printf("Client closed connection to Event Source: #%u connected from %s\n", client->socket(), client->remoteIP().toString());
    });
    _server->on(EVENT_NOTIFICATION_SERVICE_PATH, &_eventSource);

    ESP_LOGV("NotificationEvents", "Registered Event Source endpoint: %s", EVENT_NOTIFICATION_SERVICE_PATH);
}

void NotificationEvents::pushNotification(String message, pushEvent event, int id)
{
    String eventType;
    switch (event)
    {
    case (PUSHERROR):
        eventType = "errorToast";
        break;
    case (PUSHWARNING):
        eventType = "warningToast";
        break;
    case (PUSHINFO):
        eventType = "infoToast";
        break;
    case (PUSHSUCCESS):
        eventType = "successToast";
        break;
    default:
        return;
    }
    _eventSource.send(message.c_str(), eventType.c_str(), id);
}

void NotificationEvents::send(String message, String event, int id)
{
    _eventSource.send(message.c_str(), event.c_str(), id);
}
