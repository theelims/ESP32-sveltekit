/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <EthernetSettingsService.h>

EthernetSettingsService::EthernetSettingsService(PsychicHttpServer *server,
                                                 FS *fs,
                                                 SecurityManager *securityManager,
                                                 EventSocket *socket) : _server(server),
                                                 _securityManager(securityManager),
                                                 _httpEndpoint(EthernetSettings::read, EthernetSettings::update, this, server, ETHERNET_SETTINGS_SERVICE_PATH, securityManager,
                                                               AuthenticationPredicates::IS_ADMIN),
                                                 _fsPersistence(EthernetSettings::read, EthernetSettings::update, this, fs, ETHERNET_SETTINGS_FILE),
                                                 _socket(socket)
{
    addUpdateHandler([&](const String &originId)
                     { reconfigureEthernet(); },
                     false);
}

void EthernetSettingsService::initEthernet()
{
    // make sure the interface is stopped before continuing and initializing
    ETH.end();
    _fsPersistence.readFromFS();
    configureNetwork(_state.ethernetSettings);
}

void EthernetSettingsService::begin()
{
    _socket->registerEvent(EVENT_ETHERNET);
    _httpEndpoint.begin();
}

void EthernetSettingsService::loop()
{
    unsigned long currentMillis = millis();

    if (!_lastEthernetUpdate || (unsigned long)(currentMillis - _lastEthernetUpdate) >= ETHERNET_EVENT_DELAY)
    {
        _lastEthernetUpdate = currentMillis;
        updateEthernet();
    }
}

String EthernetSettingsService::getHostname()
{
    return _state.hostname;
}

String EthernetSettingsService::getIP()
{
    if (ETH.connected())
    {
        return ETH.localIP().toString();
    }
    return "Not connected";
}

void EthernetSettingsService::configureNetwork(ethernet_settings_t &network)
{
    // set hostname before IP configuration starts
    ETH.setHostname(_state.hostname.c_str());
    if (network.staticIPConfig)
    {
        // configure for static IP
        ETH.config(network.localIP, network.gatewayIP, network.subnetMask, network.dnsIP1, network.dnsIP2);
    }
    else
    {
        // configure for DHCP
        ETH.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    }
    // (re)start ethernet
    ETH.begin();
    // set hostname (again) after (re)starting ethernet due to a bug in the ESP-IDF implementation
    ETH.setHostname(_state.hostname.c_str());

}

void EthernetSettingsService::reconfigureEthernet()
{
    configureNetwork(_state.ethernetSettings);
}

void EthernetSettingsService::updateEthernet()
{
    JsonDocument doc;
    doc["connected"] = ETH.connected();
    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_ETHERNET, jsonObject);
}