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

#include <LightMqttSettingsService.h>

LightMqttSettingsService::LightMqttSettingsService(PsychicHttpServer *server,
                                                   ESP32SvelteKit *sveltekit) : _httpEndpoint(LightMqttSettings::read,
                                                                                              LightMqttSettings::update,
                                                                                              this,
                                                                                              server,
                                                                                              LIGHT_BROKER_SETTINGS_PATH,
                                                                                              sveltekit->getSecurityManager(),
                                                                                              AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                _fsPersistence(LightMqttSettings::read,
                                                                                               LightMqttSettings::update,
                                                                                               this,
                                                                                               sveltekit->getFS(),
                                                                                               LIGHT_BROKER_SETTINGS_FILE),
                                                                                _mqttSettingsService(sveltekit->getMqttSettingsService())
{
    // configure settings service update handler to update LED state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
}

void LightMqttSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}

void LightMqttSettingsService::onConfigUpdated()
{
    // Notify the MQTT client about the updated configuration
    _mqttSettingsService->setStatusTopic(_state.stateTopic);

    // Optionally, you can also log or handle the updated configuration here
    ESP_LOGI(LIGHT_TAG, "MQTT Configuration updated");
}
