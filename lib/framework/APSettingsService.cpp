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

#include <APSettingsService.h>

APSettingsService::APSettingsService(PsychicHttpServer *server,
                                     FS *fs,
                                     SecurityManager *securityManager) : _server(server),
                                                                         _securityManager(securityManager),
                                                                         _httpEndpoint(APSettings::read, APSettings::update, this, server, AP_SETTINGS_SERVICE_PATH, securityManager),
                                                                         _fsPersistence(APSettings::read, APSettings::update, this, fs, AP_SETTINGS_FILE),
                                                                         _dnsServer(nullptr),
                                                                         _lastManaged(0),
                                                                         _reconfigureAp(false)
{
    addUpdateHandler([&](const String &originId)
                     { reconfigureAP(); },
                     false);
}

void APSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
    reconfigureAP();
}

void APSettingsService::reconfigureAP()
{
    _lastManaged = millis() - MANAGE_NETWORK_DELAY;
    _reconfigureAp = true;
    _recoveryMode = false;
}

void APSettingsService::recoveryMode()
{
#ifdef SERIAL_INFO
    Serial.println("Recovery Mode needed");
#endif
    _lastManaged = millis() - MANAGE_NETWORK_DELAY;
    _recoveryMode = true;
    _reconfigureAp = true;
}

void APSettingsService::loop()
{
    unsigned long currentMillis = millis();
    unsigned long manageElapsed = (unsigned long)(currentMillis - _lastManaged);
    if (manageElapsed >= MANAGE_NETWORK_DELAY)
    {
        _lastManaged = currentMillis;
        manageAP();
    }
    handleDNS();
}

void APSettingsService::manageAP()
{
    WiFiMode_t currentWiFiMode = WiFi.getMode();
    if (_state.provisionMode == AP_MODE_ALWAYS ||
        (_state.provisionMode == AP_MODE_DISCONNECTED && WiFi.status() != WL_CONNECTED) || _recoveryMode)
    {
        if (_reconfigureAp || currentWiFiMode == WIFI_OFF || currentWiFiMode == WIFI_STA)
        {
            startAP();
        }
    }
    else if ((currentWiFiMode == WIFI_AP || currentWiFiMode == WIFI_AP_STA) &&
             (_reconfigureAp || !WiFi.softAPgetStationNum()))
    {
        stopAP();
    }
    _reconfigureAp = false;
}

void APSettingsService::startAP()
{
#ifdef SERIAL_INFO
    Serial.println("Starting software access point");
#endif
    WiFi.softAPConfig(_state.localIP, _state.gatewayIP, _state.subnetMask);
    WiFi.softAP(_state.ssid.c_str(), _state.password.c_str(), _state.channel, _state.ssidHidden, _state.maxClients);
#if CONFIG_IDF_TARGET_ESP32C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // https://www.wemos.cc/en/latest/c3/c3_mini_1_0_0.html#about-wifi
#endif
    if (!_dnsServer)
    {
        IPAddress apIp = WiFi.softAPIP();
#ifdef SERIAL_INFO
        Serial.print("Starting captive portal on ");
        Serial.println(apIp);
#endif
        _dnsServer = new DNSServer;
        _dnsServer->start(DNS_PORT, "*", apIp);
    }
}

void APSettingsService::stopAP()
{
    if (_dnsServer)
    {
#ifdef SERIAL_INFO
        Serial.println("Stopping captive portal");
#endif
        _dnsServer->stop();
        delete _dnsServer;
        _dnsServer = nullptr;
    }
#ifdef SERIAL_INFO
    Serial.println("Stopping software access point");
#endif
    WiFi.softAPdisconnect(true);
}

void APSettingsService::handleDNS()
{
    if (_dnsServer)
    {
        _dnsServer->processNextRequest();
    }
}

APNetworkStatus APSettingsService::getAPNetworkStatus()
{
    WiFiMode_t currentWiFiMode = WiFi.getMode();
    bool apActive = currentWiFiMode == WIFI_AP || currentWiFiMode == WIFI_AP_STA;
    if (apActive && _state.provisionMode != AP_MODE_ALWAYS && WiFi.status() == WL_CONNECTED)
    {
        return APNetworkStatus::LINGERING;
    }
    return apActive ? APNetworkStatus::ACTIVE : APNetworkStatus::INACTIVE;
}
