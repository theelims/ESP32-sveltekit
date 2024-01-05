/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <StrokeEngineSafetyService.h>

StrokeEngineSafetyService::StrokeEngineSafetyService(StrokeEngine *stroker, AsyncWebServer *server, FS *fs, SecurityManager *securityManager, StrokeEngineControlService *strokeEngineControlService) : _strokeEngine(stroker),
                                                                                                                                                                                                        _httpEndpoint(StrokeEngineSafety::read,
                                                                                                                                                                                                                      StrokeEngineSafety::update,
                                                                                                                                                                                                                      this,
                                                                                                                                                                                                                      server,
                                                                                                                                                                                                                      SAFETY_CONFIG_PATH,
                                                                                                                                                                                                                      securityManager,
                                                                                                                                                                                                                      AuthenticationPredicates::NONE_REQUIRED),
                                                                                                                                                                                                        _fsPersistence(StrokeEngineSafety::read, StrokeEngineSafety::update, this, fs, SAFETY_CONFIG_FILE),
                                                                                                                                                                                                        _strokeEngineControlService(strokeEngineControlService)
{
    // configure settings service update handler to update state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(originId); },
                     false);
}

void StrokeEngineSafetyService::begin()
{
    // read from FS
    _fsPersistence.readFromFS();
    ESP_LOGI("StrokeEngineSafetyService", "Read safety settings from FS");

    // Sanity check of values just read from FS
    if (_state.depthLimit < 0.0 || _state.strokeLimit < 0.0)
    {
        // update stroke engine safety settings with default values
        update([&](StrokeEngineSafety &state)
               { 
                state.depthLimit = _strokeEngine->getMotor()->getMaxPosition();
                state.strokeLimit = _strokeEngine->getMotor()->getMaxPosition();
                return StateUpdateResult::CHANGED; },
               "initialization");
    }
    else
    {
        // update stroke engine safety settings
        onConfigUpdated("begin");
    }
}

void StrokeEngineSafetyService::onConfigUpdated(String originId)
{

    ESP_LOGI("StrokeEngineSafetyService", "Update safety settings by %s", originId.c_str());
    boolean sanitized = false;

    // update stroke engine safety settings
    float depthLimit = _strokeEngine->setLimit(StrokeLimit::DEPTH, _state.depthLimit);
    if (depthLimit != _state.depthLimit)
    {
        _state.depthLimit = depthLimit;
        sanitized = true;
    }

    float strokeLimit = _strokeEngine->setLimit(StrokeLimit::STROKE, _state.strokeLimit);
    if (strokeLimit != _state.strokeLimit)
    {
        _state.strokeLimit = strokeLimit;
        sanitized = true;
    }

    float rateLimit = _strokeEngine->setLimit(StrokeLimit::RATE, _state.rateLimit);
    if (rateLimit != _state.rateLimit)
    {
        _state.rateLimit = rateLimit;
        sanitized = true;
    }

    // Apply new values immediately
    _strokeEngine->applyChangesNow();

    // TODO implement heartbeat mode
    // _strokeEngine->setEaseInSpeed(_state.easeInSpeed);

    // update stroke engine control service
    //_strokeEngineControlService->setHeartbeatMode(_state.heartbeatMode);

    // Propagate sanitized changes to StatefulService, but prevent infinite loop
    if (sanitized && originId != "onConfigUpdated")
    {
        ESP_LOGW("StrokeEngineSafetyService", "Sanitized safety settings");
        update([&](StrokeEngineSafety &state)
               { return StateUpdateResult::CHANGED; },
               "onConfigUpdated");
    }
}