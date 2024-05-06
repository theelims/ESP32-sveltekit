#ifndef LightMqttSettingsService_h
#define LightMqttSettingsService_h

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
#include <SettingValue.h>

#define LIGHT_BROKER_SETTINGS_FILE "/config/brokerSettings.json"
#define LIGHT_BROKER_SETTINGS_PATH "/rest/brokerSettings"

class LightMqttSettings
{
public:
    String mqttPath;
    String name;
    String uniqueId;

    static void read(LightMqttSettings &settings, JsonObject &root)
    {
        root["mqtt_path"] = settings.mqttPath;
        root["name"] = settings.name;
        root["unique_id"] = settings.uniqueId;
    }

    static StateUpdateResult update(JsonObject &root, LightMqttSettings &settings)
    {
        settings.mqttPath = root["mqtt_path"] | SettingValue::format("homeassistant/light/#{unique_id}");
        settings.name = root["name"] | SettingValue::format("light-#{unique_id}");
        settings.uniqueId = root["unique_id"] | SettingValue::format("light-#{unique_id}");
        return StateUpdateResult::CHANGED;
    }
};

class LightMqttSettingsService : public StatefulService<LightMqttSettings>
{
public:
    LightMqttSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager);
    void begin();

private:
    HttpEndpoint<LightMqttSettings> _httpEndpoint;
    FSPersistence<LightMqttSettings> _fsPersistence;
};

#endif // end LightMqttSettingsService_h
