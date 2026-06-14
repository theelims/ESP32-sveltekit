#pragma once

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2025 theelims
 *   Copyright (C) 2026 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <HomeAssistant/HAService.h>
#include <SettingValue.h>

#ifndef FACTORY_HA_ENABLED
#define FACTORY_HA_ENABLED false
#endif

#ifndef FACTORY_HA_DISCOVERY_PREFIX
#define FACTORY_HA_DISCOVERY_PREFIX "homeassistant/"
#endif

#ifndef FACTORY_HA_DEVICE_NAME
#define FACTORY_HA_DEVICE_NAME ""
#endif

#ifndef FACTORY_HA_MANUFACTURER
#define FACTORY_HA_MANUFACTURER ""
#endif

#ifndef FACTORY_HA_MODEL
#define FACTORY_HA_MODEL ""
#endif

#define HA_SETTINGS_FILE "/config/haSettings.json"
#define HA_SETTINGS_SERVICE_PATH "/rest/haSettings"

class HASettings
{
public:
    bool enabled;
    String discoveryPrefix;
    String deviceName;
    String manufacturer;
    String model;

    static void read(HASettings &settings, JsonObject &root)
    {
        root["enabled"] = settings.enabled;
        root["discovery_prefix"] = settings.discoveryPrefix;
        root["device_name"] = settings.deviceName;
        root["manufacturer"] = settings.manufacturer;
        root["model"] = settings.model;
    }

    static StateUpdateResult update(JsonObject &root, HASettings &settings, const String &originId)
    {
        settings.enabled = root["enabled"] | FACTORY_HA_ENABLED;
        settings.discoveryPrefix = root["discovery_prefix"] | FACTORY_HA_DISCOVERY_PREFIX;
        settings.deviceName = root["device_name"] | SettingValue::format(FACTORY_HA_DEVICE_NAME);
        settings.manufacturer = root["manufacturer"] | FACTORY_HA_MANUFACTURER;
        settings.model = root["model"] | FACTORY_HA_MODEL;

        if (!settings.discoveryPrefix.endsWith("/"))
        {
            settings.discoveryPrefix += "/";
        }
        return StateUpdateResult::CHANGED;
    }
};

/**
 * @brief Home Assistant settings service
 *
 * Persists user-facing HA configuration (enabled, discovery prefix, device
 * identity) and bridges it to the framework HAService. On every settings
 * change the new values are pushed into HAService and — if MQTT is connected
 * — a full republish is triggered so HA discovery picks up the change.
 *
 * An empty device_name falls back to APP_NAME so that the field can be left
 * blank in the UI to use the firmware's compile-time name.
 */
class HASettingsService : public StatefulService<HASettings>
{
public:
    static constexpr const char *TAG = "HASettings";

    HASettingsService(PsychicHttpServer *server,
                      FS *fs,
                      SecurityManager *securityManager,
                      HAService *haService);

    void begin();

private:
    HttpEndpoint<HASettings> _httpEndpoint;
    FSPersistence<HASettings> _fsPersistence;
    HAService *_haService;

    void _applyToHAService();
};
