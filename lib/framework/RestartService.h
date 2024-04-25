#ifndef RestartService_h
#define RestartService_h

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

#include <PsychicHttp.h>
#include <SecurityManager.h>

#define RESTART_SERVICE_PATH "/rest/restart"

class RestartService
{
public:
    RestartService(PsychicHttpServer *server, SecurityManager *securityManager);

    void begin();

    static void restartNow()
    {
        WiFi.disconnect(true);
        delay(500);
        ESP.restart();
    }

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    esp_err_t restart(PsychicRequest *request);
};

#endif // end RestartService_h
