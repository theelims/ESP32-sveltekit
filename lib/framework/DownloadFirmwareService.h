#pragma once

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

#include <Arduino.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

#include <ESPAsyncWebServer.h>
#include <SecurityManager.h>
#include <NotificationEvents.h>

#include <HTTPClient.h>
#include <HTTPUpdate.h>

#define GITHUB_FIRMWARE_PATH "/rest/downloadUpdate"
#define MAX_GITHUB_SIZE 192
#define OTA_TASK_STACK_SIZE 9216

class DownloadFirmwareService
{
public:
    DownloadFirmwareService(AsyncWebServer *server, SecurityManager *securityManager, NotificationEvents *notificationEvents);

private:
    SecurityManager *_securityManager;
    AsyncCallbackJsonWebHandler _downloadHandler;
    void downloadUpdate(AsyncWebServerRequest *request, JsonVariant &json);
};
