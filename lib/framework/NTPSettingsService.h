#ifndef NTPSettingsService_h
#define NTPSettingsService_h

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

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <WiFi.h>

#include <time.h>
#include <lwip/apps/sntp.h>

#ifndef FACTORY_NTP_ENABLED
#define FACTORY_NTP_ENABLED true
#endif

#ifndef FACTORY_NTP_TIME_ZONE_LABEL
#define FACTORY_NTP_TIME_ZONE_LABEL "Europe/London"
#endif

#ifndef FACTORY_NTP_TIME_ZONE_FORMAT
#define FACTORY_NTP_TIME_ZONE_FORMAT "GMT0BST,M3.5.0/1,M10.5.0"
#endif

#ifndef FACTORY_NTP_SERVER
#define FACTORY_NTP_SERVER "time.google.com"
#endif

#define NTP_SETTINGS_FILE "/config/ntpSettings.json"
#define NTP_SETTINGS_SERVICE_PATH "/rest/ntpSettings"

#define TIME_PATH "/rest/time"

class NTPSettings
{
public:
    bool enabled;
    String tzLabel;
    String tzFormat;
    String server;

    static void read(NTPSettings &settings, JsonObject &root)
    {
        root["enabled"] = settings.enabled;
        root["server"] = settings.server;
        root["tz_label"] = settings.tzLabel;
        root["tz_format"] = settings.tzFormat;
    }

    static StateUpdateResult update(JsonObject &root, NTPSettings &settings)
    {
        settings.enabled = root["enabled"] | FACTORY_NTP_ENABLED;
        settings.server = root["server"] | FACTORY_NTP_SERVER;
        settings.tzLabel = root["tz_label"] | FACTORY_NTP_TIME_ZONE_LABEL;
        settings.tzFormat = root["tz_format"] | FACTORY_NTP_TIME_ZONE_FORMAT;
        return StateUpdateResult::CHANGED;
    }
};

class NTPSettingsService : public StatefulService<NTPSettings>
{
public:
    NTPSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager);

    void begin();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    HttpEndpoint<NTPSettings> _httpEndpoint;
    FSPersistence<NTPSettings> _fsPersistence;

    void onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    void onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    void configureNTP();
    esp_err_t configureTime(PsychicRequest *request, JsonVariant &json);
};

#endif // end NTPSettingsService_h
