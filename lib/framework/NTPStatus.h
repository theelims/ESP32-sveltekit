#ifndef NTPStatus_h
#define NTPStatus_h

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

#include <time.h>
#include <WiFi.h>
#include <lwip/apps/sntp.h>

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <SecurityManager.h>

#define NTP_STATUS_SERVICE_PATH "/rest/ntpStatus"

class NTPStatus
{
public:
    NTPStatus(AsyncWebServer *server, SecurityManager *securityManager);

    void begin();

private:
    AsyncWebServer *_server;
    SecurityManager *_securityManager;
    void ntpStatus(AsyncWebServerRequest *request);
};

#endif // end NTPStatus_h
