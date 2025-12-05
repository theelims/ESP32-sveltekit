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

#include <ESP32SvelteKit.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include <PsychicHttpServer.h>
#include <StatusMonitor.h>
#include "Controller.h"

#define SERIAL_BAUD_RATE 115200

PsychicHttpServer server;

ESP32SvelteKit esp32sveltekit(&server, 70);

LightMqttSettingsService lightMqttSettingsService = LightMqttSettingsService(&server,
                                                                             &esp32sveltekit);

LightStateService lightStateService = LightStateService(&server,
                                                        &esp32sveltekit,
                                                        &lightMqttSettingsService);

StatusMonitor statusMonitor = StatusMonitor(&esp32sveltekit);
void setup()
{
    // start serial and filesystem
    Serial.begin(SERIAL_BAUD_RATE);

    controller.setup();

    // start ESP32-SvelteKit
    esp32sveltekit.begin();
    statusMonitor.begin();

    // load the initial light settings
    lightStateService.begin();
    // start the light service
    lightMqttSettingsService.begin();

    // Add loop callbacks to ESP32-SvelteKit
    esp32sveltekit.addLoopFunction([]() { 
        statusMonitor.loop(); 
        controller.loop();

        //every second
        static unsigned long lastSecond = 0;
        if (millis() - lastSecond >= 1000) {
            lastSecond = millis();
        }

        //every 10 seconds
        static unsigned long last10Second = 0;
        if (millis() - last10Second >= 10000) {
            last10Second = millis();
            
        }
    });
}

void loop()
{
    // Delete Arduino loop task, as it is not needed in this example
    vTaskDelete(NULL);
}
