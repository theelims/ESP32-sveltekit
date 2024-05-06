#ifndef MqttStatus_h
#define MqttStatus_h

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

#include <MqttSettingsService.h>
#include <ArduinoJson.h>
#include <PsychicHttp.h>
#include <SecurityManager.h>

#define MQTT_STATUS_SERVICE_PATH "/rest/mqttStatus"

class MqttStatus
{
public:
    MqttStatus(PsychicHttpServer *server, MqttSettingsService *mqttSettingsService, SecurityManager *securityManager);

    void begin();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    MqttSettingsService *_mqttSettingsService;

    esp_err_t mqttStatus(PsychicRequest *request);
};

#endif // end MqttStatus_h
