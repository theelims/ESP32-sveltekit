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

#include <WiFiSettingsService.h>

WiFiSettingsService::WiFiSettingsService(PsychicHttpServer *server,
                                         FS *fs,
                                         SecurityManager *securityManager,
                                         EventSocket *socket) : _server(server),
                                                                _securityManager(securityManager),
                                                                _httpEndpoint(WiFiSettings::read, WiFiSettings::update, this, server, WIFI_SETTINGS_SERVICE_PATH, securityManager,
                                                                              AuthenticationPredicates::IS_ADMIN),
                                                                _fsPersistence(WiFiSettings::read, WiFiSettings::update, this, fs, WIFI_SETTINGS_FILE), _lastConnectionAttempt(0),
                                                                _socket(socket)
{
    addUpdateHandler([&](const String &originId)
                     { reconfigureWiFiConnection(); },
                     false);
}

void WiFiSettingsService::initWiFi()
{
    WiFi.mode(WIFI_MODE_STA); // this is the default.

    // Disable WiFi config persistance and auto reconnect
    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);

    WiFi.onEvent(
        std::bind(&WiFiSettingsService::onStationModeDisconnected, this, std::placeholders::_1, std::placeholders::_2),
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(std::bind(&WiFiSettingsService::onStationModeStop, this, std::placeholders::_1, std::placeholders::_2),
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_STOP);

    _fsPersistence.readFromFS();
    reconfigureWiFiConnection();
}

void WiFiSettingsService::begin()
{
    _socket->registerEvent(EVENT_RSSI);

    _httpEndpoint.begin();
}

void WiFiSettingsService::reconfigureWiFiConnection()
{
    // reset last connection attempt to force loop to reconnect immediately
    _lastConnectionAttempt = 0;

    // disconnect and de-configure wifi
    if (WiFi.disconnect(true))
    {
        _stopping = true;
    }
}

void WiFiSettingsService::loop()
{
    unsigned long currentMillis = millis();
    if (!_lastConnectionAttempt || (unsigned long)(currentMillis - _lastConnectionAttempt) >= WIFI_RECONNECTION_DELAY)
    {
        _lastConnectionAttempt = currentMillis;
        manageSTA();
    }

    if (!_lastRssiUpdate || (unsigned long)(currentMillis - _lastRssiUpdate) >= RSSI_EVENT_DELAY)
    {
        _lastRssiUpdate = currentMillis;
        updateRSSI();
    }
}

String WiFiSettingsService::getHostname()
{
    return _state.hostname;
}

void WiFiSettingsService::manageSTA()
{
    // Abort if already connected, or if we have no SSID
    if (WiFi.isConnected() || _state.wifiSettings.empty())
    {
        return;
    }

    // Connect or reconnect as required
    if ((WiFi.getMode() & WIFI_STA) == 0)
    {
#ifdef SERIAL_INFO
        Serial.println("Connecting to WiFi...");
#endif
        connectToWiFi();
    }
}

void WiFiSettingsService::connectToWiFi()
{
    // reset availability flag for all stored networks
    for (auto &network : _state.wifiSettings)
    {
        network.available = false;
    }

    // scanning for available networks
    int scanResult = WiFi.scanNetworks();
    if (scanResult == WIFI_SCAN_FAILED)
    {
        ESP_LOGE("WiFiSettingsService", "WiFi scan failed.");
    }
    else if (scanResult == 0)
    {
        ESP_LOGW("WiFiSettingsService", "No networks found.");
    }
    else
    {
        ESP_LOGI("WiFiSettingsService", "%d networks found.", scanResult);

        // find the best network to connect
        wifi_settings_t *bestNetwork = NULL;
        int bestNetworkDb = FACTORY_WIFI_RSSI_THRESHOLD;

        for (int i = 0; i < scanResult; ++i)
        {
            String ssid_scan;
            int32_t rssi_scan;
            uint8_t sec_scan;
            uint8_t *BSSID_scan;
            int32_t chan_scan;

            WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan);
            ESP_LOGV("WiFiSettingsService", "SSID: %s, BSSID: " MACSTR ", RSSI: %d dbm, Channel: %d", ssid_scan.c_str(), MAC2STR(BSSID_scan), rssi_scan, chan_scan);

            for (auto &network : _state.wifiSettings)
            {
                if (ssid_scan == network.ssid)
                { // SSID match
                    if (rssi_scan > bestNetworkDb)
                    { // best network
                        bestNetworkDb = rssi_scan;
                        ESP_LOGV("WiFiSettingsService", "--> New best network SSID: %s, BSSID: " MACSTR "", ssid_scan.c_str(), MAC2STR(BSSID_scan));
                        network.available = true;
                        network.channel = chan_scan;
                        memcpy(network.bssid, BSSID_scan, 6);
                        bestNetwork = &network;
                    }
                    else if (rssi_scan >= FACTORY_WIFI_RSSI_THRESHOLD && network.available == false)
                    { // available network
                        network.available = true;
                        network.channel = chan_scan;
                        memcpy(network.bssid, BSSID_scan, 6);
                    }
                }
                break;
            }
        }

        // if configured to prioritize signal strength, use the best network else use the first available network
        if (_state.priorityBySignalStrength == false)
        {
            for (auto &network : _state.wifiSettings)
            {
                if (network.available == true)
                {
                    ESP_LOGI("WiFiSettingsService", "Connecting to first available network: %s", network.ssid.c_str());
                    configureNetwork(network);
                    break;
                }
            }
        }
        else if (_state.priorityBySignalStrength == true && bestNetwork)
        {
            ESP_LOGI("WiFiSettingsService", "Connecting to strongest network: %s, BSSID: " MACSTR " ", bestNetwork->ssid.c_str(), MAC2STR(bestNetwork->bssid));
            configureNetwork(*bestNetwork);
        }
        else // no suitable network to connect
        {
            ESP_LOGI("WiFiSettingsService", "No known networks found.");
        }

        // delete scan results
        WiFi.scanDelete();
    }
}

void WiFiSettingsService::configureNetwork(wifi_settings_t &network)
{
    if (network.staticIPConfig)
    {
        // configure for static IP
        WiFi.config(network.localIP, network.gatewayIP, network.subnetMask, network.dnsIP1, network.dnsIP2);
    }
    else
    {
        // configure for DHCP
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    }
    WiFi.setHostname(_state.hostname.c_str());

    // attempt to connect to the network
    WiFi.begin(network.ssid.c_str(), network.password.c_str(), network.channel, network.bssid);
    // WiFi.begin(network.ssid.c_str(), network.password.c_str());

#if CONFIG_IDF_TARGET_ESP32C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // https://www.wemos.cc/en/latest/c3/c3_mini_1_0_0.html#about-wifi
#endif
}

void WiFiSettingsService::updateRSSI()
{
    JsonDocument doc;
    doc["rssi"] = WiFi.RSSI();
    doc["ssid"] = WiFi.isConnected() ? WiFi.SSID() : "disconnected";
    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_RSSI, jsonObject);
}

void WiFiSettingsService::onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    WiFi.disconnect(true);
}
void WiFiSettingsService::onStationModeStop(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (_stopping)
    {
        _lastConnectionAttempt = 0;
        _stopping = false;
    }
}
