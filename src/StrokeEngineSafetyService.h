#pragma once

/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <HttpEndpoint.h>
#include <StrokeEngine.h>
#include <StrokeEngineControlService.h>

#ifndef MOTION_FACTORY_EASE_IN_SPEED
#define MOTION_FACTORY_EASE_IN_SPEED 20.0
#endif

#ifndef MOTION_MAX_RATE
#define MOTION_MAX_RATE 240.0
#endif

#define SAFETY_CONFIG_PATH "/rest/safety"
#define SAFETY_CONFIG_FILE "/config/safetyConfig.json"

class StrokeEngineSafety
{
public:
    float depthLimit;
    float strokeLimit;
    float rateLimit;
    HeartbeatMode heartbeatMode;
    float easeInSpeed;

    static void read(StrokeEngineSafety &settings, JsonObject &root)
    {
        root["depthLimit"] = settings.depthLimit;
        root["strokeLimit"] = settings.strokeLimit;
        root["rateLimit"] = settings.rateLimit;
        root["heartbeatMode"] = settings.heartbeatMode;
        root["easeInSpeed"] = settings.easeInSpeed;
    }

    static StateUpdateResult update(JsonObject &root, StrokeEngineSafety &settings)
    {
        settings.depthLimit = root["depthLimit"] | -1.0;
        settings.strokeLimit = root["strokeLimit"] | -1.0;
        settings.rateLimit = root["rateLimit"] | MOTION_MAX_RATE;
        settings.heartbeatMode = root["heartbeatMode"] | HeartbeatMode::NONE;
        settings.easeInSpeed = root["easeInSpeed"] | MOTION_FACTORY_EASE_IN_SPEED;

        return StateUpdateResult::CHANGED;
    }
};

class StrokeEngineSafetyService : public StatefulService<StrokeEngineSafety>
{
public:
    StrokeEngineSafetyService(StrokeEngine *stroker,
                              AsyncWebServer *server,
                              FS *fs,
                              SecurityManager *securityManager,
                              StrokeEngineControlService *strokeEngineControlService);

    void begin();

private:
    HttpEndpoint<StrokeEngineSafety> _httpEndpoint;
    FSPersistence<StrokeEngineSafety> _fsPersistence;
    StrokeEngineControlService *_strokeEngineControlService;
    StrokeEngine *_strokeEngine;

    void onConfigUpdated(String originId);
};