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
                                                       AsyncWebServer *server,
                                                       SecurityManager *securityManager,
                                                       AsyncMqttClient *mqttClient,
                                                       LightMqttSettingsService *lightMqttSettingsService) : _strokeEngine(strokeEngine),
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
                                                                                                             _lightMqttSettingsService(lightMqttSettingsService)
/*  _webSocketClient(StrokeEngineControl::read,
StrokeEngineControl::update,
this,
SE_CONTROL_SETTINGS_SOCKET_PATH)*/
{
    // configure MQTT callback
    _mqttClient->onConnect(std::bind(&StrokeEngineControlService::registerConfig, this));

    // configure update handler for when the light settings change
    _lightMqttSettingsService->addUpdateHandler([&](const String &originId)
                                                { registerConfig(); },
                                                false);

    // configure settings service update handler to update state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
}

void StrokeEngineControlService::begin()
{
    _state.go = false;
    _state.depth = MOTION_FACTORY_TRAVEL;
    _state.stroke = MOTION_FACTORY_STROKE;
    _state.rate = MOTION_FACTORY_RATE;
    _state.sensation = MOTION_FACTORY_SENSATION;
    _state.pattern = MOTION_FACTORY_PATTERN;
    _state.vibrationOverride = false;
    _state.vibrationAmplitude = MOTION_FACTORY_VIBRATION_AMPLITUDE;
    _state.vibrationFrequency = MOTION_FACTORY_VIBRATION_FREQUENCY;
}

void StrokeEngineControlService::onConfigUpdated()
{
    ESP_LOGI("StrokeEngineControlService", "Config updated");
    // Update stroke parameters
    _state.depth = _strokeEngine->setParameter(StrokeParameter::DEPTH, _state.depth, true);
    _state.stroke = _strokeEngine->setParameter(StrokeParameter::STROKE, _state.stroke, true);
    _state.rate = _strokeEngine->setParameter(StrokeParameter::RATE, _state.rate, true);
    _state.sensation = _strokeEngine->setParameter(StrokeParameter::SENSATION, _state.sensation, true);

    // only update pattern, if it has changed
    if (_strokeEngine->getCurrentPatternName() != _state.pattern)
    {
        _strokeEngine->setPattern(_state.pattern, true);
    }

    // Change running state of the stroke engine
    if ((_state.go == true) && (_strokeEngine->isActive() == false))
    {
        _strokeEngine->startPattern();
    }
    else if ((_state.go == false) && (_strokeEngine->isActive() == true))
    {
        _strokeEngine->stopMotion();
    }
}

void StrokeEngineControlService::registerConfig()
{
    if (!_mqttClient->connected())
    {
        return;
    }
    String configTopic;
    String subTopic;
    String pubTopic;

    DynamicJsonDocument doc(256);
    _lightMqttSettingsService->read([&](LightMqttSettings &settings)
                                    {
    configTopic = settings.mqttPath + "/config";
    subTopic = settings.mqttPath + "/set";
    pubTopic = settings.mqttPath + "/state";
    doc["~"] = settings.mqttPath;
    doc["name"] = settings.name;
    doc["unique_id"] = settings.uniqueId; });
    doc["cmd_t"] = "~/set";
    doc["stat_t"] = "~/state";
    doc["schema"] = "json";
    doc["brightness"] = false;

    String payload;
    serializeJson(doc, payload);
    _mqttClient->publish(configTopic.c_str(), 0, false, payload.c_str());

    _mqttPubSub.configureTopics(pubTopic, subTopic);
}
