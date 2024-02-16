/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <StrokeEngineEnvironmentService.h>

StrokeEngineEnvironmentService::StrokeEngineEnvironmentService(StrokeEngine *strokeEngine,
                                                               PsychicHttpServer *server,
                                                               MotorConfigurationService *motorConfigurationService,
                                                               StrokeEngineSafetyService *strokeEngineSafetyService,
                                                               SecurityManager *securityManager,
                                                               PsychicMqttClient *mqttClient,
                                                               MqttBrokerSettingsService *mqttBrokerSettingsService) : _strokeEngine(strokeEngine),
                                                                                                                       _motorConfigurationService(motorConfigurationService),
                                                                                                                       _strokeEngineSafetyService(strokeEngineSafetyService),
                                                                                                                       _server(server),
                                                                                                                       _securityManager(securityManager),
                                                                                                                       _mqttClient(mqttClient),
                                                                                                                       _mqttBrokerSettingsService(mqttBrokerSettingsService)
{
}

void StrokeEngineEnvironmentService::begin()
{
    _server->on(ENVIRONMENT_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&StrokeEngineEnvironmentService::environment, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));

    String environmentTopic;
    _mqttBrokerSettingsService->read([&](MqttBrokerSettings &settings)
                                     { environmentTopic = settings.environmentTopic; });

    // publish the environment on every change of a broker setting
    _mqttBrokerSettingsService->addUpdateHandler([&](const String &originId)
                                                 { mqttPublishEnvironment(); },
                                                 false);

    // publish environment on connect and as reply to a message on the environment topic
    _mqttClient->onTopic(environmentTopic.c_str(), 1, std::bind(&StrokeEngineEnvironmentService::mqttPingBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    _mqttClient->onConnect(std::bind(&StrokeEngineEnvironmentService::mqttPublishEnvironment, this, std::placeholders::_1));
}

void StrokeEngineEnvironmentService::mqttPingBack(const char *topic, const char *payload, int retain, int qos, bool dup)
{
    if (strcmp(payload, "environment") == 0)
        mqttPublishEnvironment();
}

void StrokeEngineEnvironmentService::mqttPublishEnvironment(bool session)
{
    // create JSON object
    StaticJsonDocument<MAX_ENVIRONMENT_SIZE> doc;
    JsonObject root = doc.to<JsonObject>();
    createEnvironmentJson(root);

    String payloadString;
    serializeJson(doc, payloadString);

    // get the environment topic
    String environmentTopic;
    _mqttBrokerSettingsService->read([&](MqttBrokerSettings &settings)
                                     { environmentTopic = settings.environmentTopic; });

    // publish the environment
    _mqttClient->publish(environmentTopic.c_str(), 1, true, payloadString.c_str());
}

void StrokeEngineEnvironmentService::createEnvironmentJson(JsonObject root)
{
    root["depth"] = _strokeEngine->getMotor()->getMaxPosition();
    root["max_rate"] = MOTION_MAX_RATE;
    root["max_velocity"] = _strokeEngine->getMotor()->getMaxSpeed();
    WatchdogMode heartbeatMode;
    _strokeEngineSafetyService->read([&](StrokeEngineSafety &safety)
                                     { heartbeatMode = safety.heartbeatMode; });
    root["heartbeat_mode"] = heartbeatMode;

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
}

esp_err_t StrokeEngineEnvironmentService::environment(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false, MAX_ENVIRONMENT_SIZE);
    JsonObject root = response.getRoot();
    createEnvironmentJson(root);
    return response.send();
}
