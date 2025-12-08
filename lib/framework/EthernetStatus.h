#ifndef EthernetStatus_h
#define EthernetStatus_h

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

#include <WiFi.h>
#include <ETH.h>

#include <ArduinoJson.h>
#include <PsychicHttp.h>
#include <IPUtils.h>
#include <SecurityManager.h>

#define ETHERNET_STATUS_SERVICE_PATH "/rest/ethernetStatus"

#if FT_ENABLED(FT_ETHERNET)

class EthernetStatus
{
public:
    EthernetStatus(PsychicHttpServer *server, SecurityManager *securityManager);

    void begin();

    bool isConnected();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;

    // static functions for logging Ethernet events to the UART
    // they are using the same signature as WiFi events
    static void onConnected(WiFiEvent_t event, WiFiEventInfo_t info);
    static void onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    static void onGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    esp_err_t ethernetStatus(PsychicRequest *request);
};

#endif // end FT_ENABLED(FT_ETHERNET)

#endif // end EthernetStatus_h
