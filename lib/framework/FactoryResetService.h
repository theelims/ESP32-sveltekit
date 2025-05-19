#ifndef FactoryResetService_h
#define FactoryResetService_h

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
#include <RestartService.h>
#include <FS.h>

#define FS_CONFIG_DIRECTORY "/config"
#define FACTORY_RESET_SERVICE_PATH "/rest/factoryReset"

class FactoryResetService
{
    FS *fs;

public:
    FactoryResetService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager);

    void begin();
    void factoryReset();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    esp_err_t handleRequest(PsychicRequest *request);
};

#endif // end FactoryResetService_h
