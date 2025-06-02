#pragma once

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <SettingValue.h>
#include <ESP32SvelteKit.h>

#define FFC_MQTT_SETTINGS_PATH "/rest/ffc-mqtt-settings"
#define FFC_MQTT_SETTINGS_FILE "/config/ffc-mqtt-settings.json"

#define FFC_MQTT_TOPIC "smarthome/fridge-fan-control/alarm"

class FFCMqttSettings
{
public:
    String topic;

    static void read(FFCMqttSettings &settings, JsonObject &root)
    {
        root["topic"] = settings.topic;

        ESP_LOGV(FFCMqttSettings::TAG, "Gateway MQTT settings read.");
    }

    static StateUpdateResult update(JsonObject &root, FFCMqttSettings &settings)
    {
        settings.topic = root["topic"] | SettingValue::format(FFC_MQTT_TOPIC);

        ESP_LOGV(FFCMqttSettings::TAG, "MQTT settings updated.");

        return StateUpdateResult::CHANGED;
    }

private:
    static constexpr const char *TAG = "FFCMqttSettings";
};

class FFCMqttSettingsService : public StatefulService<FFCMqttSettings>
{
public:
    FFCMqttSettingsService(ESP32SvelteKit *sveltekit);

    void begin();

    String getTopic()
    {
        beginTransaction();
        String topic = _state.topic;
        endTransaction();

        return topic;
    }

private:
    HttpEndpoint<FFCMqttSettings> _httpEndpoint;
    FSPersistence<FFCMqttSettings> _fsPersistence;
};
