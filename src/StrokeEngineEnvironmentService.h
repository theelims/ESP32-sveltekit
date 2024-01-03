#pragma once
/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <StrokeEngine.h>
#include <MotorConfigurationService.h>

#define MAX_ENVIRONMENT_SIZE 512
#define ENVIRONMENT_SERVICE_PATH "/rest/environment"

class StrokeEngineEnvironmentService
{
public:
    StrokeEngineEnvironmentService(StrokeEngine *strokeEngine, AsyncWebServer *server, MotorConfigurationService *motorConfigurationService);

private:
    StrokeEngine *_strokeEngine;
    MotorConfigurationService *_motorConfigurationService;
    void environment(AsyncWebServerRequest *request);
};
