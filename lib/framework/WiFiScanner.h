#ifndef WiFiScanner_h
#define WiFiScanner_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
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
#include <SecurityManager.h>

#define SCAN_NETWORKS_SERVICE_PATH "/rest/scanNetworks"
#define LIST_NETWORKS_SERVICE_PATH "/rest/listNetworks"

#define MAX_WIFI_SCANNER_SIZE 1024

class WiFiScanner
{
public:
    WiFiScanner(AsyncWebServer *server, SecurityManager *securityManager);

private:
    void scanNetworks(AsyncWebServerRequest *request);
    void listNetworks(AsyncWebServerRequest *request);
};

#endif // end WiFiScanner_h
