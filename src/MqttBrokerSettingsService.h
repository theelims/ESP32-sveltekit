#ifndef MqttBrokerSettingsService_h
#define MqttBrokerSettingsService_h

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
#include <SettingValue.h>

#define MQTT_BROKER_SETTINGS_FILE "/config/brokerSettings.json"
#define MQTT_BROKER_SETTINGS_PATH "/rest/brokerSettings"

class MqttBrokerSettings
{
public:
    String controlTopic;
    String environmentTopic;
    String streamingTopic;

    static void read(MqttBrokerSettings &settings, JsonObject &root)
    {
        root["control_topic"] = settings.controlTopic;
        root["environment_topic"] = settings.environmentTopic;
        root["streaming_topic"] = settings.streamingTopic;
    }

    static StateUpdateResult update(JsonObject &root, MqttBrokerSettings &settings)
    {
        settings.controlTopic = root["control_topic"] | SettingValue::format("lust-motion/#{unique_id}/control");
        settings.environmentTopic = root["environment_topic"] | SettingValue::format("lust-motion/#{unique_id}/environment");
        settings.streamingTopic = root["streaming_topic"] | SettingValue::format("lust-motion/#{unique_id}/streaming");
        return StateUpdateResult::CHANGED;
    }
};

class MqttBrokerSettingsService : public StatefulService<MqttBrokerSettings>
{
public:
    MqttBrokerSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager);
    void begin();

private:
    HttpEndpoint<MqttBrokerSettings> _httpEndpoint;
    FSPersistence<MqttBrokerSettings> _fsPersistence;
};

#endif // end MqttBrokerSettingsService_h
