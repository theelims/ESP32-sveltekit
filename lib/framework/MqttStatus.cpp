/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <MqttStatus.h>

MqttStatus::MqttStatus(PsychicHttpServer *server,
                       MqttSettingsService *mqttSettingsService,
                       SecurityManager *securityManager) : _server(server),
                                                           _securityManager(securityManager),
                                                           _mqttSettingsService(mqttSettingsService)
{
}

void MqttStatus::begin()
{
    _server->on(MQTT_STATUS_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&MqttStatus::mqttStatus, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));

    ESP_LOGV("MqttStatus", "Registered GET endpoint: %s", MQTT_STATUS_SERVICE_PATH);
}

esp_err_t MqttStatus::mqttStatus(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject root = response.getRoot();

    root["enabled"] = _mqttSettingsService->isEnabled();
    root["connected"] = _mqttSettingsService->isConnected();
    root["client_id"] = _mqttSettingsService->getClientId();
    root["last_error"] = _mqttSettingsService->getLastError();

    return response.send();
}
