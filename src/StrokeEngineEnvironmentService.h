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
#include <PsychicMqttClient.h>
#include <SecurityManager.h>
#include <StrokeEngine.h>
#include <MotorConfigurationService.h>
#include <MqttBrokerSettingsService.h>

#define MAX_ENVIRONMENT_SIZE 512
#define ENVIRONMENT_SERVICE_PATH "/rest/environment"

class StrokeEngineEnvironmentService
{
public:
    StrokeEngineEnvironmentService(StrokeEngine *strokeEngine, PsychicHttpServer *server, MotorConfigurationService *motorConfigurationService, SecurityManager *securityManager, PsychicMqttClient *mqttClient, MqttBrokerSettingsService *mqttBrokerSettingsService);

    void begin();

private:
    StrokeEngine *_strokeEngine;
    MotorConfigurationService *_motorConfigurationService;
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    PsychicMqttClient *_mqttClient;
    MqttBrokerSettingsService *_mqttBrokerSettingsService;

    void mqttPingBack(const char *topic, const char *payload, int retain, int qos, bool dup);
    void mqttPublishEnvironment(bool session = true);
    void createEnvironmentJson(JsonObject root);
    esp_err_t environment(PsychicRequest *request);
};
