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
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Arduino.h>

#include <Update.h>
#include <WiFi.h>

#include <PsychicHttp.h>
#include <SecurityManager.h>
#include <RestartService.h>

#define UPLOAD_FIRMWARE_PATH "/rest/uploadFirmware"

enum FileType
{
    ft_none = 0,
    ft_firmware = 1,
    ft_md5 = 2
};

class UploadFirmwareService
{
public:
    UploadFirmwareService(PsychicHttpServer *server, SecurityManager *securityManager);

    void begin();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;

    esp_err_t handleUpload(PsychicRequest *request,
                           const String &filename,
                           uint64_t index,
                           uint8_t *data,
                           size_t len,
                           bool final);
    esp_err_t uploadComplete(PsychicRequest *request);
    esp_err_t handleError(PsychicRequest *request, int code);
    esp_err_t handleEarlyDisconnect();
};

#endif // end UploadFirmwareService_h
