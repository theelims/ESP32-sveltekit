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

#include <APStatus.h>

APStatus::APStatus(PsychicHttpServer *server,
                   SecurityManager *securityManager,
                   APSettingsService *apSettingsService) : _server(server),
                                                           _securityManager(securityManager),
                                                           _apSettingsService(apSettingsService)
{
}
void APStatus::begin()
{
    _server->on(AP_STATUS_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&APStatus::apStatus, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));

    ESP_LOGV("APStatus", "Registered GET endpoint: %s", AP_STATUS_SERVICE_PATH);
}

esp_err_t APStatus::apStatus(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject root = response.getRoot();

    root["status"] = _apSettingsService->getAPNetworkStatus();
    root["ip_address"] = WiFi.softAPIP().toString();
    root["mac_address"] = WiFi.softAPmacAddress();
    root["station_num"] = WiFi.softAPgetStationNum();

    return response.send();
}
