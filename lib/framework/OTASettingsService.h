#ifndef OTASettingsService_h
#define OTASettingsService_h

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

#include <HttpEndpoint.h>
#include <FSPersistence.h>

#include <ESPmDNS.h>

#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#ifndef FACTORY_OTA_PORT
#define FACTORY_OTA_PORT 8266
#endif

#ifndef FACTORY_OTA_PASSWORD
#define FACTORY_OTA_PASSWORD "esp-sveltekit"
#endif

#ifndef FACTORY_OTA_ENABLED
#define FACTORY_OTA_ENABLED true
#endif

#define OTA_SETTINGS_FILE "/config/otaSettings.json"
#define OTA_SETTINGS_SERVICE_PATH "/rest/otaSettings"

class OTASettings
{
public:
    bool enabled;
    int port;
    String password;

    static void read(OTASettings &settings, JsonObject &root)
    {
        root["enabled"] = settings.enabled;
        root["port"] = settings.port;
        root["password"] = settings.password;
    }

    static StateUpdateResult update(JsonObject &root, OTASettings &settings)
    {
        settings.enabled = root["enabled"] | FACTORY_OTA_ENABLED;
        settings.port = root["port"] | FACTORY_OTA_PORT;
        settings.password = root["password"] | FACTORY_OTA_PASSWORD;
        return StateUpdateResult::CHANGED;
    }
};

class OTASettingsService : public StatefulService<OTASettings>
{
public:
    OTASettingsService(AsyncWebServer *server, FS *fs, SecurityManager *securityManager);

    void begin();

private:
    HttpEndpoint<OTASettings> _httpEndpoint;
    FSPersistence<OTASettings> _fsPersistence;
    ArduinoOTAClass *_arduinoOTA;

    void configureArduinoOTA();
    void onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info);

    static void _loopImpl(void *_this) { static_cast<OTASettingsService *>(_this)->_loop(); }
    void _loop();
    TaskHandle_t _loopHandle = NULL;
};

#endif // end OTASettingsService_h
