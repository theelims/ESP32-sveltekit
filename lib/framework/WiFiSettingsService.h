#ifndef WiFiSettingsService_h
#define WiFiSettingsService_h

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

#include <WiFi.h>
#include <WiFiMulti.h>
#include <SettingValue.h>
#include <StatefulService.h>
#include <EventSocket.h>
#include <FSPersistence.h>
#include <HttpEndpoint.h>
#include <JsonUtils.h>
#include <SecurityManager.h>
#include <PsychicHttp.h>
#include <vector>

#ifndef FACTORY_WIFI_SSID
#define FACTORY_WIFI_SSID ""
#endif

#ifndef FACTORY_WIFI_PASSWORD
#define FACTORY_WIFI_PASSWORD ""
#endif

#ifndef FACTORY_WIFI_HOSTNAME
#define FACTORY_WIFI_HOSTNAME "#{platform}-#{unique_id}"
#endif

#ifndef FACTORY_WIFI_RSSI_THRESHOLD
#define FACTORY_WIFI_RSSI_THRESHOLD -80
#endif

#define WIFI_SETTINGS_FILE "/config/wifiSettings.json"
#define WIFI_SETTINGS_SERVICE_PATH "/rest/wifiSettings"

#define WIFI_RECONNECTION_DELAY 1000 * 30
#define RSSI_EVENT_DELAY 500

#define EVENT_RSSI "rssi"

// Struct defining the wifi settings
typedef struct
{
    String ssid;
    uint8_t bssid[6];
    int32_t channel;
    String password;
    bool staticIPConfig;
    IPAddress localIP;
    IPAddress gatewayIP;
    IPAddress subnetMask;
    IPAddress dnsIP1;
    IPAddress dnsIP2;
    bool available;
} wifi_settings_t;

class WiFiSettings
{
public:
    // core wifi configuration
    String hostname;
    bool priorityBySignalStrength;
    std::vector<wifi_settings_t> wifiSettings;

    static void read(WiFiSettings &settings, JsonObject &root)
    {
        root["hostname"] = settings.hostname;
        root["priority_RSSI"] = settings.priorityBySignalStrength;

        // create JSON array from root
        JsonArray wifiNetworks = root["wifi_networks"].to<JsonArray>();

        // iterate over the wifiSettings
        for (auto &wifi : settings.wifiSettings)
        {
            // create JSON object for each wifi network
            JsonObject wifiNetwork = wifiNetworks.add<JsonObject>();

            // add the ssid and password to the JSON object
            wifiNetwork["ssid"] = wifi.ssid;
            wifiNetwork["password"] = wifi.password;
            wifiNetwork["static_ip_config"] = wifi.staticIPConfig;

            // extended settings
            JsonUtils::writeIP(root, "local_ip", wifi.localIP);
            JsonUtils::writeIP(root, "gateway_ip", wifi.gatewayIP);
            JsonUtils::writeIP(root, "subnet_mask", wifi.subnetMask);
            JsonUtils::writeIP(root, "dns_ip_1", wifi.dnsIP1);
            JsonUtils::writeIP(root, "dns_ip_2", wifi.dnsIP2);
        }

        ESP_LOGV("WiFiSettings", "WiFi Settings read");
    }

    static StateUpdateResult update(JsonObject &root, WiFiSettings &settings)
    {
        settings.hostname = root["hostname"] | SettingValue::format(FACTORY_WIFI_HOSTNAME);
        settings.priorityBySignalStrength = root["priority_RSSI"] | true;

        settings.wifiSettings.clear();

        // create JSON array from root
        JsonArray wifiNetworks = root["wifi_networks"];
        if (root["wifi_networks"].is<JsonArray>())
        {
            // iterate over the wifiSettings
            int i = 0;
            for (auto wifiNetwork : wifiNetworks)
            {
                // max 5 wifi networks
                if (i++ >= 5)
                {
                    ESP_LOGE("WiFiSettings", "Too many wifi networks");
                    break;
                }

                // create JSON object for each wifi network
                JsonObject wifi = wifiNetwork.as<JsonObject>();

                // Check if SSID length is between 1 and 31 characters and password between 0 and 64 characters
                if (wifi["ssid"].as<String>().length() < 1 || wifi["ssid"].as<String>().length() > 31 || wifi["password"].as<String>().length() > 64)
                {
                    ESP_LOGE("WiFiSettings", "SSID or password length is invalid");
                }
                else
                {
                    // add the ssid and password to the JSON object
                    wifi_settings_t wifiSettings;

                    wifiSettings.ssid = wifi["ssid"].as<String>();
                    wifiSettings.password = wifi["password"].as<String>();
                    wifiSettings.staticIPConfig = wifi["static_ip_config"];

                    // extended settings
                    JsonUtils::readIP(wifi, "local_ip", wifiSettings.localIP);
                    JsonUtils::readIP(wifi, "gateway_ip", wifiSettings.gatewayIP);
                    JsonUtils::readIP(wifi, "subnet_mask", wifiSettings.subnetMask);
                    JsonUtils::readIP(wifi, "dns_ip_1", wifiSettings.dnsIP1);
                    JsonUtils::readIP(wifi, "dns_ip_2", wifiSettings.dnsIP2);

                    // Swap around the dns servers if 2 is populated but 1 is not
                    if (IPUtils::isNotSet(wifiSettings.dnsIP1) && IPUtils::isSet(wifiSettings.dnsIP2))
                    {
                        wifiSettings.dnsIP1 = wifiSettings.dnsIP2;
                        wifiSettings.dnsIP2 = INADDR_NONE;
                    }

                    // Turning off static ip config if we don't meet the minimum requirements
                    // of ipAddress, gateway and subnet. This may change to static ip only
                    // as sensible defaults can be assumed for gateway and subnet
                    if (wifiSettings.staticIPConfig && (IPUtils::isNotSet(wifiSettings.localIP) || IPUtils::isNotSet(wifiSettings.gatewayIP) ||
                                                        IPUtils::isNotSet(wifiSettings.subnetMask)))
                    {
                        wifiSettings.staticIPConfig = false;
                    }

                    // reset scan result
                    wifiSettings.available = false;
                    settings.wifiSettings.push_back(wifiSettings);

                    // increment the wifi network index
                    i++;
                }
            }
        }
        else
        {
            // populate with factory defaults if they are present
            if (String(FACTORY_WIFI_SSID).length() > 0)
            {
                settings.wifiSettings.push_back(wifi_settings_t{
                    .ssid = FACTORY_WIFI_SSID,
                    .password = FACTORY_WIFI_PASSWORD,
                    .staticIPConfig = false,
                    .localIP = INADDR_NONE,
                    .gatewayIP = INADDR_NONE,
                    .subnetMask = INADDR_NONE,
                    .dnsIP1 = INADDR_NONE,
                    .dnsIP2 = INADDR_NONE,
                    .available = false,
                });
            }
        }
        ESP_LOGV("WiFiSettings", "WiFi Settings updated");

        return StateUpdateResult::CHANGED;
    };
};

class WiFiSettingsService : public StatefulService<WiFiSettings>
{
public:
    WiFiSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager, EventSocket *socket);

    void initWiFi();
    void begin();
    void loop();
    String getHostname();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    HttpEndpoint<WiFiSettings> _httpEndpoint;
    FSPersistence<WiFiSettings> _fsPersistence;
    EventSocket *_socket;
    unsigned long _lastConnectionAttempt;
    unsigned long _lastRssiUpdate;

    bool _stopping;
    void onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    void onStationModeStop(WiFiEvent_t event, WiFiEventInfo_t info);

    void reconfigureWiFiConnection();
    void manageSTA();
    void connectToWiFi();
    void configureNetwork(wifi_settings_t &network);
    void updateRSSI();
};

#endif // end WiFiSettingsService_h
