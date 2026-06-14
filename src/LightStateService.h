#ifndef LightStateService_h
#define LightStateService_h

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

#include <EventSocket.h>
#include <HttpEndpoint.h>
#include <EventEndpoint.h>
#include <WebSocketServer.h>
#include <ESP32SvelteKit.h>
#include <HomeAssistant/HALight.h>
#include <LightSettingsService.h>

#define DEFAULT_LED_STATE false
#define OFF_STATE "OFF"
#define ON_STATE "ON"

#define LIGHT_SETTINGS_ENDPOINT_PATH "/rest/lightState"
#define LIGHT_SETTINGS_SOCKET_PATH "/ws/lightState"
#define LIGHT_SETTINGS_EVENT "led"

// PWM / fade parameters
#define LED_PWM_FREQ_HZ      5000
#define LED_PWM_BITS         8      // 8-bit → duty 0–255
#define LED_FADE_DURATION_MS 1000
#define LED_FADE_STEPS       100    // one step every 10 ms

// LED_BUILTIN on ESP32-S3 DevKitC resolves to a virtual NeoPixel pin:
//   RGB_BUILTIN = SOC_GPIO_PIN_COUNT (49) + 48 = 97
// The preprocessor mis-evaluates this because SOC_GPIO_PIN_COUNT is undefined
// at #if time, so use this macro only in C++ `if()` expressions — never in
// #if directives.  The C++ compiler resolves LED_BUILTIN to 97 correctly and
// eliminates the dead branch at compile time.
#define LED_HW_AVAILABLE (LED_BUILTIN < 64)

class LightState
{
public:
    bool ledOn;

    static void read(LightState &settings, JsonObject &root)
    {
        root["led_on"] = settings.ledOn;
    }

    static StateUpdateResult update(JsonObject &root, LightState &lightState, const String &originID)
    {
        boolean newState = root["led_on"] | DEFAULT_LED_STATE;
        if (lightState.ledOn != newState)
        {
            lightState.ledOn = newState;
            return StateUpdateResult::CHANGED;
        }
        return StateUpdateResult::UNCHANGED;
    }

    static void homeAssistRead(LightState &settings, JsonObject &root)
    {
        root["state"] = settings.ledOn ? ON_STATE : OFF_STATE;
    }

    static StateUpdateResult homeAssistUpdate(JsonObject &root, LightState &lightState, const String &originID)
    {
        String state = root["state"];
        boolean newState = false;
        if (state.equals(ON_STATE))
        {
            newState = true;
        }
        else if (!state.equals(OFF_STATE))
        {
            return StateUpdateResult::ERROR;
        }
        if (lightState.ledOn != newState)
        {
            lightState.ledOn = newState;
            return StateUpdateResult::CHANGED;
        }
        return StateUpdateResult::UNCHANGED;
    }
};

class LightStateService : public StatefulService<LightState>
{
public:
    LightStateService(PsychicHttpServer *server,
                      ESP32SvelteKit *sveltekit,
                      LightSettingsService *lightSettings);

    void begin();

private:
    HttpEndpoint<LightState>    _httpEndpoint;
    EventEndpoint<LightState>   _eventEndpoint;
    WebSocketServer<LightState> _webSocketServer;
    LightSettingsService        *_lightSettings;

    // Fade state
    volatile bool _fadeCancelled = false;
    TaskHandle_t  _fadeTask      = nullptr;
    uint8_t       _fadeTarget    = 0;

    void onConfigUpdated();
    static void _fadeTaskImpl(void *param);
};

#endif
