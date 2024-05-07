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

#include <WiFiScanner.h>

WiFiScanner::WiFiScanner(PsychicHttpServer *server,
                         SecurityManager *securityManager) : _server(server),
                                                             _securityManager(securityManager)
{
}

void WiFiScanner::begin()
{
    _server->on(SCAN_NETWORKS_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&WiFiScanner::scanNetworks, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_ADMIN));

    ESP_LOGV("WiFiScanner", "Registered GET endpoint: %s", SCAN_NETWORKS_SERVICE_PATH);

    _server->on(LIST_NETWORKS_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&WiFiScanner::listNetworks, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_ADMIN));

    ESP_LOGV("WiFiScanner", "Registered GET endpoint: %s", LIST_NETWORKS_SERVICE_PATH);
}

esp_err_t WiFiScanner::scanNetworks(PsychicRequest *request)
{
    if (WiFi.scanComplete() != -1)
    {
        WiFi.scanDelete();
        WiFi.scanNetworks(true);
    }
    return request->reply(202);
}

esp_err_t WiFiScanner::listNetworks(PsychicRequest *request)
{
    int numNetworks = WiFi.scanComplete();
    if (numNetworks > -1)
    {
        PsychicJsonResponse response = PsychicJsonResponse(request, false);
        JsonObject root = response.getRoot();
        JsonArray networks = root["networks"].to<JsonArray>();
        for (int i = 0; i < numNetworks; i++)
        {
            JsonObject network = networks.add<JsonObject>();
            network["rssi"] = WiFi.RSSI(i);
            network["ssid"] = WiFi.SSID(i);
            network["bssid"] = WiFi.BSSIDstr(i);
            network["channel"] = WiFi.channel(i);
            network["encryption_type"] = (uint8_t)WiFi.encryptionType(i);
        }

        return response.send();
    }
    else if (numNetworks == -1)
    {
        return request->reply(202);
    }
    else
    {
        return scanNetworks(request);
    }
}
