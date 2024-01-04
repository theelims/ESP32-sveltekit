/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <StrokeEngineEnvironmentService.h>

StrokeEngineEnvironmentService::StrokeEngineEnvironmentService(StrokeEngine *strokeEngine, AsyncWebServer *server, MotorConfigurationService *motorConfigurationService) : _strokeEngine(strokeEngine),
                                                                                                                                                                           _motorConfigurationService(motorConfigurationService)
{
    server->on(ENVIRONMENT_SERVICE_PATH, HTTP_GET, std::bind(&StrokeEngineEnvironmentService::environment, this, std::placeholders::_1));
}

void StrokeEngineEnvironmentService::environment(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse(false, MAX_ENVIRONMENT_SIZE);
    JsonObject root = response->getRoot();
    root["depth"] = _strokeEngine->getMotor()->getMaxPosition();
    root["max_rate"] = MOTION_MAX_RATE;
    root["heartbeat"] = false; // TODO implement heartbeat in StrokeEngineControlService

    // create a new array for patterns
    JsonArray patterns = root.createNestedArray("patterns");
    // add the patterns
    for (int i = 0; i < _strokeEngine->getNumberOfPattern(); i++)
    {
        patterns.add(_strokeEngine->getPatternName(i));
    }
    root["valueA"] = _strokeEngine->getMotor()->getMotionPointLabel().labelValueA;
    root["valueB"] = _strokeEngine->getMotor()->getMotionPointLabel().labelValueB;
    root["motor"] = _motorConfigurationService->getDriverName();

    response->setLength();
    request->send(response);
}
