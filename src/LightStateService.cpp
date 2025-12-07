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

#include <LightStateService.h>

LightStateService::LightStateService(PsychicHttpServer *server,
                                     ESP32SvelteKit *sveltekit,
                                     LightMqttSettingsService *lightMqttSettingsService) : _httpEndpoint(LightState::read,
                                                                                                         LightState::update,
                                                                                                         this,
                                                                                                         server,
                                                                                                         LIGHT_SETTINGS_ENDPOINT_PATH,
                                                                                                         sveltekit->getSecurityManager(),
                                                                                                         AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                           _eventEndpoint(LightState::read,
                                                                                                          LightState::update,
                                                                                                          this,
                                                                                                          sveltekit->getSocket(),
                                                                                                          LIGHT_SETTINGS_EVENT),
                                                                                           _mqttEndpoint(LightState::read,
                                                                                                         LightState::update,
                                                                                                         this,
                                                                                                         sveltekit->getMqttClient()),
                                                                                           _mqttClient(sveltekit->getMqttClient()),
                                                                                           _lightMqttSettingsService(lightMqttSettingsService)
{
    // configure led to be output
    pinMode(LED_BUILTIN, OUTPUT);

    // configure settings service update handler to update LED state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
    // update the shock topic when the broker settings change
    _lightMqttSettingsService->addUpdateHandler([&](const String &originId)
                                                { configureMqttTopics(); },
                                                false);
}

void LightStateService::begin()
{
    neopixelStrip.Begin();
    _httpEndpoint.begin();
    _eventEndpoint.begin();
    onConfigUpdated();
}

void LightStateService::onConfigUpdated()
{
    // Update the color of each section of the LED strip
    // first section: 0-9
    // second section: 10-14
    // third section: 15-24
    // fourth section: 25-29
    for (size_t i = 0; i < _state.sections.size(); i++)
    {
        size_t startIndex = 0;
        size_t endIndex = 0;
        switch (i)
        {
        case 0:
            startIndex = 0;
            endIndex = 9;
            break;
        case 1:
            startIndex = 10;
            endIndex = 14;
            break;
        case 2:
            startIndex = 15;
            endIndex = 24;
            break;
        case 3:
            startIndex = 25;
            endIndex = 29;
            break;
        default:
            break;
        }

        for (size_t j = startIndex; j <= endIndex; j++)
        {
            neopixelStrip.SetPixelColor(j, RgbwwColor(_state.sections[i].red,
                                                      _state.sections[i].green,
                                                      _state.sections[i].blue,
                                                      _state.sections[i].warmWhite,
                                                      _state.sections[i].coldWhite));
        }
    }
    neopixelStrip.Show();
}

void LightStateService::configureMqttTopics()
{
    String controlTopicPub;
    String controlTopicSub;
    _lightMqttSettingsService->read([&](LightMqttSettings &settings)
                                    {
                                        controlTopicPub = settings.mqttSetPath;
                                        controlTopicSub = settings.mqttStatusPath; });
    _mqttEndpoint.configureTopics(controlTopicPub.c_str(), controlTopicSub.c_str());
}
