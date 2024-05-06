#ifndef APStatus_h
#define APStatus_h

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
#include <IPAddress.h>
#include <SecurityManager.h>
#include <APSettingsService.h>

#define AP_STATUS_SERVICE_PATH "/rest/apStatus"

class APStatus
{
public:
    APStatus(PsychicHttpServer *server, SecurityManager *securityManager, APSettingsService *apSettingsService);

    void begin();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    APSettingsService *_apSettingsService;
    esp_err_t apStatus(PsychicRequest *request);
};

#endif // end APStatus_h
