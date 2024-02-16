/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <StrokeEngineControlService.h>

StrokeEngineControlService::StrokeEngineControlService(StrokeEngine *strokeEngine,
                                                       PsychicHttpServer *server,
                                                       SecurityManager *securityManager,
                                                       PsychicMqttClient *mqttClient,
                                                       MqttBrokerSettingsService *mqttBrokerSettingsService) : _strokeEngine(strokeEngine),
                                                                                                               _httpEndpoint(StrokeEngineControl::read,
                                                                                                                             StrokeEngineControl::update,
                                                                                                                             this,
                                                                                                                             server,
                                                                                                                             SE_CONTROL_SETTINGS_ENDPOINT_PATH,
                                                                                                                             securityManager,
                                                                                                                             AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                                               _mqttPubSub(StrokeEngineControl::read, StrokeEngineControl::update, this, mqttClient),
                                                                                                               _webSocketServer(StrokeEngineControl::read,
                                                                                                                                StrokeEngineControl::update,
                                                                                                                                this,
                                                                                                                                server,
                                                                                                                                SE_CONTROL_SETTINGS_SOCKET_PATH,
                                                                                                                                securityManager,
                                                                                                                                AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                                               _mqttClient(mqttClient),
                                                                                                               _mqttBrokerSettingsService(mqttBrokerSettingsService),
                                                                                                               _heartbeatWatchdog(1200)
/*  _webSocketClient(StrokeEngineControl::read,
StrokeEngineControl::update,
this,
SE_CONTROL_SETTINGS_SOCKET_PATH)*/
{
    // configure settings service update handler to update state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(originId); },
                     false);
}

void StrokeEngineControlService::begin()
{
    _httpEndpoint.begin();
    _webSocketServer.begin();

    String controlTopic;
    _mqttBrokerSettingsService->read([&](MqttBrokerSettings &settings)
                                     { controlTopic = settings.controlTopic; });
    _mqttPubSub.configureTopics(controlTopic.c_str(), controlTopic.c_str());

    _state.command = "STOP";
    _state.depth = _strokeEngine->getParameter(StrokeParameter::DEPTH);
    _state.stroke = _strokeEngine->getParameter(StrokeParameter::STROKE);
    _state.rate = _strokeEngine->getParameter(StrokeParameter::RATE);
    _state.sensation = _strokeEngine->getParameter(StrokeParameter::SENSATION);
    _state.pattern = _strokeEngine->getCurrentPatternName();
    _state.vibrationOverride = false;
    _state.vibrationAmplitude = MOTION_FACTORY_VIBRATION_AMPLITUDE;
    _state.vibrationFrequency = MOTION_FACTORY_VIBRATION_FREQUENCY;

    // configure hook for watchdog
    addHookHandler([&](const String &originId, StateUpdateResult &result)
                   { 
                        // trigger watchdog if originId is not "Watchdog" or "onConfigUpdated"
                        if (originId != "Watchdog" && originId != "onConfigUpdated")
                            _heartbeatWatchdog.heartbeat(originId); },
                   false);

    _heartbeatWatchdog.onWatchdog([&](const String &originId)
                                  { watchdogTriggered(originId); });
}

void StrokeEngineControlService::onConfigUpdated(String originId)
{
    ESP_LOGI("StrokeEngineControlService", "Config updated");
    bool sanitized = false;

    // Update stroke parameters and propagate changes to StatefulService
    float depth = _strokeEngine->setParameter(StrokeParameter::DEPTH, _state.depth);
    if (depth != _state.depth)
    {
        _state.depth = depth;
        sanitized = true;
    }
    float stroke = _strokeEngine->setParameter(StrokeParameter::STROKE, _state.stroke);
    if (stroke != _state.stroke)
    {
        _state.stroke = stroke;
        sanitized = true;
    }
    float rate = _strokeEngine->setParameter(StrokeParameter::RATE, _state.rate);
    if (rate != _state.rate)
    {
        _state.rate = rate;
        sanitized = true;
    }
    float sensation = _strokeEngine->setParameter(StrokeParameter::SENSATION, _state.sensation);
    if (sensation != _state.sensation)
    {
        _state.sensation = sensation;
        sanitized = true;
    }

    // Apply new values immediately
    _strokeEngine->applyChangesNow();

    // only update pattern, if it has changed
    if (_strokeEngine->getCurrentPatternName() != _state.pattern)
    {
        _strokeEngine->setPattern(_state.pattern, true);
    }

    // Change running state of the stroke engine
    if ((_state.command.equalsIgnoreCase("playpattern")) && (_strokeEngine->isActive() == false))
    {
        _strokeEngine->runCommand(StrokeCommand::PATTERN);
    }
    else if ((_state.command.equalsIgnoreCase("STOP")))
    {
        _strokeEngine->runCommand(StrokeCommand::STOP);
    }
    else if (_state.command.equalsIgnoreCase("retract"))
    {
        _strokeEngine->runCommand(StrokeCommand::RETRACT);
        _strokeEngine->updateFixedPosition();
    }
    else if (_state.command.equalsIgnoreCase("stroke"))
    {
        _strokeEngine->runCommand(StrokeCommand::STROKE);
        _strokeEngine->updateFixedPosition();
    }
    else if (_state.command.equalsIgnoreCase("depth"))
    {
        _strokeEngine->runCommand(StrokeCommand::DEPTH);
        _strokeEngine->updateFixedPosition();
    }

    // propagate sanitized changes to StatefulService if necessary but prevent infinite loop
    if (sanitized && originId != "onConfigUpdated")
    {
        ESP_LOGI("StrokeEngineControlService", "Sanitized control settings");
        update([&](StrokeEngineControl &state)
               { return StateUpdateResult::CHANGED; },
               "onConfigUpdated");
    }
}

void StrokeEngineControlService::setHeartbeatMode(WatchdogMode mode)
{
    _heartbeatWatchdog.setWatchdogMode(mode);
}

void StrokeEngineControlService::watchdogTriggered(String originId)
{
    ESP_LOGW("StrokeEngineControlService", "Watchdog triggered [%s]- Stopping StrokeEngine", originId.c_str());
    _state.command = "STOP";
    update([&](StrokeEngineControl &state)
           { return StateUpdateResult::CHANGED; },
           "Watchdog");
}
