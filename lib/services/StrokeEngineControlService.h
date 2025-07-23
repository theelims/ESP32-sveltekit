#pragma once

/**
 *   LUST-motion
 *
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <ESP32SvelteKit.h>
#include <PsychicHttp.h>
#include <PsychicMqttClient.h>
#include <MqttEndpoint.h>
#include <WebSocketServer.h>
#include <HttpEndpoint.h>
#include <EventEndpoint.h>
#include <FSPersistence.h>
#include <JsonUtils.h>
#include <StrokeEngine.h>
#include <MqttBrokerSettingsService.h>
#include <HeartbeatWatchdog.h>

#ifdef OSSM_REF
#include <OSSMReferenceBoard.h>
// else throw compile error
#else
#error "No board defined"
#endif

#ifndef MOTION_FACTORY_TRAVEL
#define MOTION_FACTORY_TRAVEL 150.0
#endif

#ifndef MOTION_FACTORY_STROKE
#define MOTION_FACTORY_STROKE 80.0
#endif

#ifndef MOTION_FACTORY_RATE
#define MOTION_FACTORY_RATE 30.0
#endif

#ifndef MOTION_FACTORY_SENSATION
#define MOTION_FACTORY_SENSATION 0.0
#endif

#ifndef MOTION_FACTORY_PATTERN
#define MOTION_FACTORY_PATTERN "PoundingTeasing"
#endif

#ifndef MOTION_FACTORY_VIBRATION_AMPLITUDE
#define MOTION_FACTORY_VIBRATION_AMPLITUDE 2.0
#endif

#ifndef MOTION_FACTORY_VIBRATION_FREQUENCY
#define MOTION_FACTORY_VIBRATION_FREQUENCY 30.0
#endif

#define SE_CONTROL_SETTINGS_ENDPOINT_PATH "/rest/control"
#define SE_CONTROL_SETTINGS_SOCKET_PATH "/ws/control"
#define SE_CONTROL_SETTINGS_EVENT "control"

// Command Lookup Table
const String strokeCommandTable[] = {
    "STOP",
    "retract",
    "depth",
    "stroke",
    "pattern",
    "strokestream",
    "positionstream",
};

class StrokeEngineControl
{
public:
    String command;
    float depth;
    float stroke;
    float rate;
    float sensation;
    String pattern;

    bool operator==(const StrokeEngineControl &settings) const
    {
        return command == settings.command && depth == settings.depth && stroke == settings.stroke &&
               rate == settings.rate && sensation == settings.sensation && pattern == settings.pattern;
    }

    static void read(StrokeEngineControl &settings, JsonObject &root)
    {
        root["command"] = settings.command;
        root["depth"] = settings.depth;
        root["stroke"] = settings.stroke;
        root["rate"] = settings.rate;
        root["sensation"] = settings.sensation;
        root["pattern"] = settings.pattern;
    }

    static StateUpdateResult update(JsonObject &root, StrokeEngineControl &settings)
    {
        StrokeEngineControl newSettings = {};
        // All parameters will be cropped or sanitized inside StrokeEngine
        newSettings.command = root["command"] | "STOP";
        newSettings.depth = root["depth"];
        newSettings.stroke = root["stroke"];
        newSettings.rate = root["rate"];
        newSettings.sensation = root["sensation"];
        newSettings.pattern = root["pattern"].as<String>();

        if (newSettings == settings)
        {
            return StateUpdateResult::UNCHANGED;
        }
        settings = newSettings;
        return StateUpdateResult::CHANGED;
    }

protected:
};

class StrokeEngineControlService : public StatefulService<StrokeEngineControl>
{
public:
    StrokeEngineControlService(StrokeEngine *strokeEngine,
                               ESP32SvelteKit *sveltekit,
                               MqttBrokerSettingsService *mqttBrokerSettingsService);

    void begin();

private:
    HttpEndpoint<StrokeEngineControl> _httpEndpoint;
    MqttEndpoint<StrokeEngineControl> _mqttEndpoint;
    EventEndpoint<StrokeEngineControl> _eventEndpoint;
    PsychicMqttClient *_mqttClient;
    StrokeEngine *_strokeEngine;
    MqttBrokerSettingsService *_mqttBrokerSettingsService;

    void onConfigUpdated(String originId);

    void onStrokeEngineChanged(String reason);
};
