#pragma once

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>

#include <ESPAsyncWebServer.h>
#include <SecurityManager.h>

#define SLEEP_SERVICE_PATH "/rest/sleep"

#ifndef WAKEUP_PIN_NUMBER
#define WAKEUP_PIN_NUMBER 0
#endif

#ifndef WAKEUP_SIGNAL
#define WAKEUP_SIGNAL 0
#endif

class SleepService
{
public:
    SleepService(AsyncWebServer *server, SecurityManager *securityManager);

    static void sleepNow();

    void attachOnSleepCallback(void (*callbackSleep)())
    {
        _callbackSleep = callbackSleep;
    }

private:
    void sleep(AsyncWebServerRequest *request);

protected:
    static void (*_callbackSleep)();
};
