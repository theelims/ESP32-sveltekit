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

#include <ArduinoJson.h>
#include <PsychicHttp.h>

#define EVENT_NOTIFICATION_SERVICE_PATH "/events"

enum pushEvent
{
    PUSHERROR,
    PUSHWARNING,
    PUSHINFO,
    PUSHSUCCESS
};

class NotificationEvents
{
protected:
    PsychicHttpServer *_server;
    PsychicEventSource _eventSource;

public:
    NotificationEvents(PsychicHttpServer *server);

    void begin();

    void pushNotification(String message, pushEvent event, int id = 0);

    void send(String message, String event, int id = 0);
};
