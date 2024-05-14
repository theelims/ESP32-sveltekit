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

#include <WiFiStatus.h>

WiFiStatus::WiFiStatus(PsychicHttpServer *server,
                       SecurityManager *securityManager) : _server(server),
                                                           _securityManager(securityManager)
{
}

void WiFiStatus::begin()
{
    _server->on(WIFI_STATUS_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&WiFiStatus::wifiStatus, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));

    ESP_LOGV("WiFiStatus", "Registered GET endpoint: %s", WIFI_STATUS_SERVICE_PATH);

    WiFi.onEvent(onStationModeConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(onStationModeDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(onStationModeGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
}

void WiFiStatus::onStationModeConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    ESP_LOGI("WiFiStatus", "WiFi Connected.");

#ifdef SERIAL_INFO
    Serial.println("WiFi Connected.");
#endif
}

void WiFiStatus::onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    ESP_LOGI("WiFiStatus", "WiFi Disconnected. Reason code=%d", info.wifi_sta_disconnected.reason);

#ifdef SERIAL_INFO
    Serial.print("WiFi Disconnected. Reason code=");
    Serial.println(info.wifi_sta_disconnected.reason);
#endif
}

void WiFiStatus::onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    ESP_LOGI("WiFiStatus", "WiFi Got IP. localIP=%s, hostName=%s", WiFi.localIP().toString().c_str(), WiFi.getHostname());
#ifdef SERIAL_INFO
    Serial.printf("WiFi Got IP. localIP=%s, hostName=%s\r\n", WiFi.localIP().toString().c_str(), WiFi.getHostname());
#endif
}

esp_err_t WiFiStatus::wifiStatus(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject root = response.getRoot();
    wl_status_t status = WiFi.status();
    root["status"] = (uint8_t)status;
    if (status == WL_CONNECTED)
    {
        root["local_ip"] = WiFi.localIP().toString();
        root["mac_address"] = WiFi.macAddress();
        root["rssi"] = WiFi.RSSI();
        root["ssid"] = WiFi.SSID();
        root["bssid"] = WiFi.BSSIDstr();
        root["channel"] = WiFi.channel();
        root["subnet_mask"] = WiFi.subnetMask().toString();
        root["gateway_ip"] = WiFi.gatewayIP().toString();
        IPAddress dnsIP1 = WiFi.dnsIP(0);
        IPAddress dnsIP2 = WiFi.dnsIP(1);
        if (IPUtils::isSet(dnsIP1))
        {
            root["dns_ip_1"] = dnsIP1.toString();
        }
        if (IPUtils::isSet(dnsIP2))
        {
            root["dns_ip_2"] = dnsIP2.toString();
        }
    }

    return response.send();
}
