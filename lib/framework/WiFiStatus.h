#ifndef WiFiStatus_h
#define WiFiStatus_h

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

#include <WiFi.h>

#include <ArduinoJson.h>
#include <PsychicHttp.h>
#include <IPUtils.h>
#include <SecurityManager.h>

#define WIFI_STATUS_SERVICE_PATH "/rest/wifiStatus"

class WiFiStatus
{
public:
    WiFiStatus(PsychicHttpServer *server, SecurityManager *securityManager);

    void begin();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;

    // static functions for logging WiFi events to the UART
    static void onStationModeConnected(WiFiEvent_t event, WiFiEventInfo_t info);
    static void onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    static void onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    esp_err_t wifiStatus(PsychicRequest *request);
};

#endif // end WiFiStatus_h
