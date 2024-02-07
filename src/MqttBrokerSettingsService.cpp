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

#include <MqttBrokerSettingsService.h>

MqttBrokerSettingsService::MqttBrokerSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager) : _httpEndpoint(MqttBrokerSettings::read,
                                                                                                                                          MqttBrokerSettings::update,
                                                                                                                                          this,
                                                                                                                                          server,
                                                                                                                                          MQTT_BROKER_SETTINGS_PATH,
                                                                                                                                          securityManager,
                                                                                                                                          AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                                                            _fsPersistence(MqttBrokerSettings::read, MqttBrokerSettings::update, this, fs, MQTT_BROKER_SETTINGS_FILE)
{
}

void MqttBrokerSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}
