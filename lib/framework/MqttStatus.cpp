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

#include <MqttStatus.h>

MqttStatus::MqttStatus(AsyncWebServer *server,
                       MqttSettingsService *mqttSettingsService,
                       SecurityManager *securityManager) : _mqttSettingsService(mqttSettingsService)
{
    server->on(MQTT_STATUS_SERVICE_PATH,
               HTTP_GET,
               securityManager->wrapRequest(std::bind(&MqttStatus::mqttStatus, this, std::placeholders::_1),
                                            AuthenticationPredicates::IS_AUTHENTICATED));
}

void MqttStatus::mqttStatus(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse(false, MAX_MQTT_STATUS_SIZE);
    JsonObject root = response->getRoot();

    root["enabled"] = _mqttSettingsService->isEnabled();
    root["connected"] = _mqttSettingsService->isConnected();
    root["client_id"] = _mqttSettingsService->getClientId();
    root["disconnect_reason"] = (uint8_t)_mqttSettingsService->getDisconnectReason();

    response->setLength();
    request->send(response);
}
