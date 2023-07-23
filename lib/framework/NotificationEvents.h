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

#include <WiFi.h>
#include <AsyncTCP.h>

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

#define EVENT_NOTIFICATION_SERVICE_PATH "/events"

enum pushEvent
{
    ERROR,
    WARNING,
    INFO,
    SUCCESS
};

class NotificationEvents
{
protected:
    AsyncEventSource _eventSource;

public:
    NotificationEvents(AsyncWebServer *server) : _eventSource(EVENT_NOTIFICATION_SERVICE_PATH)
    {
        server->addHandler(&_eventSource);
        _eventSource.onConnect([&](AsyncEventSourceClient *client) { // client->send("hello", NULL, millis(), 1000);
            Serial.printf("New client connected to Event Source: %i Clients connected\n", _eventSource.count());
        });
    };

    void pushNotification(String message, pushEvent event, int id = 0)
    {
        String eventType;
        switch (event)
        {
        case (ERROR):
            eventType = "errorToast";
            break;
        case (WARNING):
            eventType = "warningToast";
            break;
        case (INFO):
            eventType = "infoToast";
            break;
        case (SUCCESS):
            eventType = "successToast";
            break;
        default:
            return;
        }
        _eventSource.send(message.c_str(), eventType.c_str(), id);
    };

    void send(String message, String event, int id = 0)
    {
        _eventSource.send(message.c_str(), event.c_str(), id);
    };
};
