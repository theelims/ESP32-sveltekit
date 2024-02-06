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
#include <PsychicHttp.h>
#include <StrokeEngine.h>
#include <MotorConfigurationService.h>

#define MAX_ENVIRONMENT_SIZE 512
#define ENVIRONMENT_SERVICE_PATH "/rest/environment"

class StrokeEngineEnvironmentService
{
public:
    StrokeEngineEnvironmentService(StrokeEngine *strokeEngine, PsychicHttp *server, MotorConfigurationService *motorConfigurationService);

    void begin();

private:
    StrokeEngine *_strokeEngine;
    MotorConfigurationService *_motorConfigurationService;
    esp_err_t environment(PsychicWebServerRequest *request);
};
