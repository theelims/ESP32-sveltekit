#ifndef UploadFirmwareService_h
#define UploadFirmwareService_h

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

#include <Update.h>
#include <WiFi.h>
#include <AsyncTCP.h>

#include <ESPAsyncWebServer.h>
#include <SecurityManager.h>
#include <RestartService.h>

#define UPLOAD_FIRMWARE_PATH "/rest/uploadFirmware"

class UploadFirmwareService
{
public:
    UploadFirmwareService(AsyncWebServer *server, SecurityManager *securityManager);

private:
    SecurityManager *_securityManager;
    void handleUpload(AsyncWebServerRequest *request,
                      const String &filename,
                      size_t index,
                      uint8_t *data,
                      size_t len,
                      bool final);
    void uploadComplete(AsyncWebServerRequest *request);
    void handleError(AsyncWebServerRequest *request, int code);
    static void handleEarlyDisconnect();
};

#endif // end UploadFirmwareService_h
