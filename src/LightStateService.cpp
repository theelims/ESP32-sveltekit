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
#include <HomeAssistant/HADevice.h>

LightStateService::LightStateService(PsychicHttpServer *server,
                                     ESP32SvelteKit *sveltekit,
                                     LightSettingsService *lightSettings)
    : _httpEndpoint(LightState::read,
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
      _webSocketServer(LightState::read,
                       LightState::update,
                       this,
                       server,
                       LIGHT_SETTINGS_SOCKET_PATH,
                       sveltekit->getSecurityManager(),
                       AuthenticationPredicates::IS_AUTHENTICATED),
      _lightSettings(lightSettings)
{
    if (LED_HW_AVAILABLE)
        ledcAttach(LED_BUILTIN, LED_PWM_FREQ_HZ, LED_PWM_BITS);
    else
        ESP_LOGW("LightStateService", "LED_BUILTIN=%d is not a usable GPIO — LED hw disabled", LED_BUILTIN);

    auto haService = sveltekit->getHAService();
    if (haService != nullptr)
    {
        auto light = std::unique_ptr<HALight<LightState>>(
            new HALight<LightState>(LightState::homeAssistRead,
                                    LightState::homeAssistUpdate,
                                    this,
                                    haService,
                                    "led",
                                    [](JsonObject &c)
                                    {
                                        c["schema"]     = "json";
                                        c["brightness"] = false;
                                    }));
        light->setName("LED").setIcon("mdi:led-on");
        haService->mainDevice().registerControl(std::move(light));
    }

    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
}

void LightStateService::begin()
{
    _httpEndpoint.begin();
    _eventEndpoint.begin();
    _state.ledOn = DEFAULT_LED_STATE;
    onConfigUpdated();
}

void LightStateService::onConfigUpdated()
{
    bool softDimming = false;
    bool activeLow   = true;
    if (_lightSettings != nullptr)
    {
        _lightSettings->read([&](LightSettings &s)
                             {
                                 softDimming = s.softDimming;
                                 activeLow   = s.activeLow;
                             });
    }

    // Signal any running fade task to stop, then wait one step for it to exit.
    _fadeCancelled = true;
    if (_fadeTask != nullptr)
    {
        // Give the task one step interval to notice the flag and exit cleanly.
        vTaskDelay(pdMS_TO_TICKS(LED_FADE_DURATION_MS / LED_FADE_STEPS + 5));
        _fadeTask = nullptr;
    }

    // activeLow=true: duty 0 = LED on, duty 255 = LED off (inverted logic)
    _fadeTarget = activeLow ? (_state.ledOn ? 0 : 255) : (_state.ledOn ? 255 : 0);

    _fadeCancelled = false;
    if (LED_HW_AVAILABLE)
    {
        if (softDimming)
            xTaskCreate(_fadeTaskImpl, "led_fade", 2048, this, 2, &_fadeTask);
        else
            ledcWrite(LED_BUILTIN, _fadeTarget);
    }
}

void LightStateService::_fadeTaskImpl(void *param)
{
    LightStateService *self = static_cast<LightStateService *>(param);

    if (LED_HW_AVAILABLE)
    {
        uint32_t current = ledcRead(LED_BUILTIN);
        uint32_t target  = self->_fadeTarget;
        int32_t  delta   = (int32_t)target - (int32_t)current;

        for (uint32_t step = 1; step <= LED_FADE_STEPS; step++)
        {
            if (self->_fadeCancelled)
                break;

            uint32_t duty = (uint32_t)((int32_t)current +
                                       delta * (int32_t)step / (int32_t)LED_FADE_STEPS);
            ledcWrite(LED_BUILTIN, duty);
            vTaskDelay(pdMS_TO_TICKS(LED_FADE_DURATION_MS / LED_FADE_STEPS));
        }
    }

    self->_fadeTask = nullptr;
    vTaskDelete(nullptr);
}
