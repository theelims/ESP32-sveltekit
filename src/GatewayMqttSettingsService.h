#pragma once

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <SettingValue.h>
#include <ESP32SvelteKit.h>

#define GATEWAY_MQTT_SETTINGS_FILE "/config/mqtt-settings.json"
#define GATEWAY_MQTT_SETTINGS_PATH "/rest/mqtt-settings"

#define GATEWAY_MQTT_TOPIC_PREFIX "homeassistant/binary_sensor/genius-"

class GatewayMqttSettings
{
public:
    String mqttPath;

    static void read(GatewayMqttSettings &settings, JsonObject &root)
    {
        root["mqtt_path"] = settings.mqttPath;

        ESP_LOGV(GatewayMqttSettings::TAG, "Gateway MQTT settings read.");
    }

    static StateUpdateResult update(JsonObject &root, GatewayMqttSettings &settings)
    {
        settings.mqttPath = root["mqtt_path"] | SettingValue::format(GATEWAY_MQTT_TOPIC_PREFIX);

        ESP_LOGV(GatewayMqttSettings::TAG, "Gateway MQTT settings updated.");

        return StateUpdateResult::CHANGED;
    }

private:
    static constexpr const char *TAG = "GatewayMqttSettings";
};

class GatewayMqttSettingsService : public StatefulService<GatewayMqttSettings>
{
public:
    GatewayMqttSettingsService(ESP32SvelteKit *sveltekit);

    void begin();

    GatewayMqttSettings &getSettings()
    {
        return _state;
    }

private:
    HttpEndpoint<GatewayMqttSettings> _httpEndpoint;
    FSPersistence<GatewayMqttSettings> _fsPersistence;
};
