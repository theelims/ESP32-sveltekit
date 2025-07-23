#pragma once
/**
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <PsychicHttp.h>
#include <PsychicMqttClient.h>
#include <MqttEndpoint.h>
#include <WebSocketServer.h>
#include <HttpEndpoint.h>
#include <EventEndpoint.h>
#include <StrokeEngine.h>
#include <MqttBrokerSettingsService.h>
#include <HeartbeatWatchdog.h>
#include <ESP32SvelteKit.h>

#define SAFE_STATE_ENDPOINT_PATH "/rest/safestate"
#define SAFE_STATE_EVENT "safestate"

static const char *safeStateTAG = "SafeStateService";

#ifndef STIM_FACTORY_WATCHDOG_TIMEOUT_MS
#define STIM_FACTORY_WATCHDOG_TIMEOUT_MS 1200
#endif

class SafeState
{
public:
    bool safeState;
    bool operator==(const SafeState &settings) const
    {
        return safeState == settings.safeState;
    }

    static void read(SafeState &settings, JsonObject &root)
    {
        root["safestate"] = settings.safeState;
    }

    static StateUpdateResult update(JsonObject &root, SafeState &settings)
    {
        SafeState newSettings = {};
        newSettings.safeState = root["safestate"] | false;

        if (newSettings == settings)
        {
            return StateUpdateResult::UNCHANGED;
        }
        settings = newSettings;
        return StateUpdateResult::CHANGED;
    }
};

class SafeStateService : public StatefulService<SafeState>
{
public:
    SafeStateService(StrokeEngine *StrokeEngine,
                     PsychicHttpServer *server,
                     ESP32SvelteKit *sveltekit,
                     MqttBrokerSettingsService *mqttBrokerSettingsService);

    void begin();

    void setHeartbeatMode(WatchdogMode mode);

private:
    StrokeEngine *_StrokeEngine;
    HttpEndpoint<SafeState> _httpEndpoint;
    EventEndpoint<SafeState> _eventEndpoint;
    MqttEndpoint<SafeState> _mqttEndpoint;
    PsychicMqttClient *_mqttClient;
    MqttBrokerSettingsService *_mqttBrokerSettingsService;
    HeartbeatWatchdog _heartbeatWatchdog;

    void onConfigUpdated(String originId);

    void onSafeStateUpdated(bool safestate);

    void watchdogTriggered(String originId);

    void configureMqttTopics();
};
