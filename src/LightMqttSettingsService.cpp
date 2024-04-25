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
                                                   FS *fs,
                                                   SecurityManager *securityManager) : _httpEndpoint(LightMqttSettings::read,
                                                                                                     LightMqttSettings::update,
                                                                                                     this,
                                                                                                     server,
                                                                                                     LIGHT_BROKER_SETTINGS_PATH,
                                                                                                     securityManager,
                                                                                                     AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                       _fsPersistence(LightMqttSettings::read,
                                                                                                      LightMqttSettings::update,
                                                                                                      this,
                                                                                                      fs,
                                                                                                      LIGHT_BROKER_SETTINGS_FILE)
{
}

void LightMqttSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}
