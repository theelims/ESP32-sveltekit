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

#include <LightMqttSettingsService.h>
#include <NeopixelBus.h>

#include <EventSocket.h>
#include <HttpEndpoint.h>
#include <MqttEndpoint.h>
#include <EventEndpoint.h>
#include <WebSocketServer.h>
#include <ESP32SvelteKit.h>
#include <vector>

#ifndef NEOPIXEL
#define NEOPIXEL 16 // GPIO pin for Neopixel data line
#endif

#ifndef STRIP_LENGTH
#define STRIP_LENGTH 30
#endif

#define DEFAULT_LED_STATE false
#define OFF_STATE "OFF"
#define ON_STATE "ON"

#define LIGHT_SETTINGS_ENDPOINT_PATH "/rest/lightState"
#define LIGHT_SETTINGS_SOCKET_PATH "/ws/lightState"
#define LIGHT_SETTINGS_EVENT "led"

// typdef color object
typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t warmWhite;
    uint8_t coldWhite;
} LightColor;

class LightState
{
public:
    // LED color vector with 4 elements
    std::vector<LightColor> sections = std::vector<LightColor>(4);

    static void read(LightState &settings, JsonObject &root)
    {
        // write each section's color values into array
        JsonArray sections = root["section"].to<JsonArray>();
        for (size_t i = 0; i < settings.sections.size(); i++)
        {
            JsonObject sectionObj = sections.add<JsonObject>();
            sectionObj["red"] = settings.sections[i].red;
            sectionObj["green"] = settings.sections[i].green;
            sectionObj["blue"] = settings.sections[i].blue;
            sectionObj["warm_white"] = settings.sections[i].warmWhite;
            sectionObj["cold_white"] = settings.sections[i].coldWhite;
        }
    }

    static StateUpdateResult update(JsonObject &root, LightState &lightState, const String &originID)
    {
        // read each section's color values from object
        if (root["section"].is<JsonArray>())
        {
            int i = 0;
            for (JsonVariant sectionObj : root["section"].as<JsonArray>())
            {
                if (i < 4)
                {
                    lightState.sections[i].red = constrain(sectionObj["red"], 0, 255) | 0;
                    lightState.sections[i].green = constrain(sectionObj["green"], 0, 255) | 0;
                    lightState.sections[i].blue = constrain(sectionObj["blue"], 0, 255) | 0;
                    lightState.sections[i].warmWhite = constrain(sectionObj["warm_white"], 0, 255) | 0;
                    lightState.sections[i].coldWhite = constrain(sectionObj["cold_white"], 0, 255) | 0;
                }
                i++;
            }
        }
        return StateUpdateResult::CHANGED;
    }
};

class LightStateService : public StatefulService<LightState>
{
public:
    LightStateService(PsychicHttpServer *server,
                      ESP32SvelteKit *sveltekit,
                      LightMqttSettingsService *lightMqttSettingsService);

    void begin();

private:
    HttpEndpoint<LightState> _httpEndpoint;
    EventEndpoint<LightState> _eventEndpoint;
    MqttEndpoint<LightState> _mqttEndpoint;
    PsychicMqttClient *_mqttClient;
    LightMqttSettingsService *_lightMqttSettingsService;
    NeoPixelBus<NeoRgbwwFeature, NeoEsp32I2s0Ws2805Method> neopixelStrip = NeoPixelBus<NeoRgbwwFeature, NeoEsp32I2s0Ws2805Method>(STRIP_LENGTH, NEOPIXEL);

    void onConfigUpdated();
    void configureMqttTopics();
};

#endif
