#ifndef EthernetSettingsService_h
#define EthernetSettingsService_h

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

#include <WiFi.h>
#include <ETH.h>
#include <SettingValue.h>
#include <StatefulService.h>
#include <EventSocket.h>
#include <FSPersistence.h>
#include <HttpEndpoint.h>
#include <JsonUtils.h>
#include <SecurityManager.h>
#include <PsychicHttp.h>
#include <vector>

#ifndef FACTORY_ETHERNET_HOSTNAME
#define FACTORY_ETHERNET_HOSTNAME "#{platform}-#{unique_id}"
#endif

#define ETHERNET_EVENT_DELAY 500

#define ETHERNET_SETTINGS_FILE "/config/ethernetSettings.json"
#define ETHERNET_SETTINGS_SERVICE_PATH "/rest/ethernetSettings"

#define EVENT_ETHERNET "ethernet"

// Struct defining the ethernet settings
typedef struct
{
    bool staticIPConfig;
    IPAddress localIP;
    IPAddress gatewayIP;
    IPAddress subnetMask;
    IPAddress dnsIP1;
    IPAddress dnsIP2;
    bool available;
} ethernet_settings_t;

class EthernetSettings
{
public:
    // core ethernet configuration
    String hostname;
    ethernet_settings_t ethernetSettings;

    static void read(EthernetSettings &settings, JsonObject &root)
    {
        root["hostname"] = settings.hostname;
        root["static_ip_config"] = settings.ethernetSettings.staticIPConfig;
        JsonUtils::writeIP(root, "local_ip", settings.ethernetSettings.localIP);
        JsonUtils::writeIP(root, "gateway_ip", settings.ethernetSettings.gatewayIP);
        JsonUtils::writeIP(root, "subnet_mask", settings.ethernetSettings.subnetMask);
        JsonUtils::writeIP(root, "dns_ip_1", settings.ethernetSettings.dnsIP1);
        JsonUtils::writeIP(root, "dns_ip_2", settings.ethernetSettings.dnsIP2);
        ESP_LOGV(SVK_TAG, "Ethernet Settings read");
    }

    static StateUpdateResult update(JsonObject &root, EthernetSettings &settings)
    {
        settings.hostname = root["hostname"] | SettingValue::format(FACTORY_ETHERNET_HOSTNAME);
        settings.ethernetSettings.staticIPConfig = root["static_ip_config"] | false;
        JsonUtils::readIP(root, "local_ip", settings.ethernetSettings.localIP);
        JsonUtils::readIP(root, "gateway_ip", settings.ethernetSettings.gatewayIP);
        JsonUtils::readIP(root, "subnet_mask", settings.ethernetSettings.subnetMask);
        JsonUtils::readIP(root, "dns_ip_1", settings.ethernetSettings.dnsIP1);
        JsonUtils::readIP(root, "dns_ip_2", settings.ethernetSettings.dnsIP2);

        // Swap around the dns servers if 2 is populated but 1 is not
        if (IPUtils::isNotSet(settings.ethernetSettings.dnsIP1) && IPUtils::isSet(settings.ethernetSettings.dnsIP2))
        {
            settings.ethernetSettings.dnsIP1 = settings.ethernetSettings.dnsIP2;
            settings.ethernetSettings.dnsIP2 = INADDR_NONE;
        }

        // Turning off static ip config if we don't meet the minimum requirements
        // of ipAddress and subnet. This may change to static ip only
        // as sensible defaults can be assumed for gateway and subnet
        if (settings.ethernetSettings.staticIPConfig && (IPUtils::isNotSet(settings.ethernetSettings.localIP) || IPUtils::isNotSet(settings.ethernetSettings.subnetMask)))
        {
            settings.ethernetSettings.staticIPConfig = false;
        }
        ESP_LOGV(SVK_TAG, "Ethernet Settings updated");

        return StateUpdateResult::CHANGED;
    };
};

class EthernetSettingsService : public StatefulService<EthernetSettings>
{
public:
    EthernetSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager, EventSocket *socket);

    void initEthernet();
    void begin();
    void loop();
    String getHostname();
    String getIP();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    HttpEndpoint<EthernetSettings> _httpEndpoint;
    FSPersistence<EthernetSettings> _fsPersistence;
    EventSocket *_socket;
    unsigned long _lastEthernetUpdate;

    void configureNetwork(ethernet_settings_t &network);
    void reconfigureEthernet();
    void updateEthernet();

};

#endif // end EthernetSettingsService_h
