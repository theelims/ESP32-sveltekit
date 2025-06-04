#pragma once

#include <JsonUtils.h>
#include <ESP32SvelteKit.h>

#define FANS_CONFIG_FILE "/config/fans-config.json"
#define FANS_CONFIG_SERVICE_PATH "/rest/fans-config"

typedef struct fan_spec_
{
    uint32_t minRPM;      // 1/min
    uint32_t maxRPM;      // 1/min
    uint32_t minPWM;      // %
    float airflow;        // cfm
    float staticPressure; // mmH2O

} fan_spec_t;

class FansConfig
{

public:
    static constexpr const char *TAG = "FansSettings";

    fan_spec_t supplyFan;
    fan_spec_t exhaustFan;

    static void read(FansConfig &fansConfig, JsonObject &root)
    {
        // Create supplyFan JSON object
        JsonObject supply = root["supplyFan"].to<JsonObject>();
        supply["minRPM"] = fansConfig.supplyFan.minRPM;
        supply["maxRPM"] = fansConfig.supplyFan.maxRPM;
        supply["minPWM"] = fansConfig.supplyFan.minPWM;
        supply["airflow"] = fansConfig.supplyFan.airflow;
        supply["staticPressure"] = fansConfig.supplyFan.staticPressure;

        // Create exhaustFan JSON object
        JsonObject exhaust = root["exhaustFan"].to<JsonObject>();
        exhaust["minRPM"] = fansConfig.exhaustFan.minRPM;
        exhaust["maxRPM"] = fansConfig.exhaustFan.maxRPM;
        exhaust["minPWM"] = fansConfig.exhaustFan.minPWM;
        exhaust["airflow"] = fansConfig.exhaustFan.airflow;
        exhaust["staticPressure"] = fansConfig.exhaustFan.staticPressure;

        ESP_LOGV(FansConfig::TAG, "Fans configuration read.");
    }

    static StateUpdateResult update(JsonObject &root, FansConfig &fansConfig)
    {
        if (root["supplyFan"].is<JsonObject>())
        {
            JsonObject supply = root["supplyFan"].as<JsonObject>();
            if (supply["minRPM"].is<uint32_t>())
                fansConfig.supplyFan.minRPM = supply["minRPM"];
            if (supply["maxRPM"].is<uint32_t>())
                fansConfig.supplyFan.maxRPM = supply["maxRPM"];
            if (supply["minPWM"].is<uint32_t>())
                fansConfig.supplyFan.minPWM = supply["minPWM"];
            if (supply["airflow"].is<float>())
                fansConfig.supplyFan.airflow = supply["airflow"];
            if (supply["staticPressure"].is<float>())
                fansConfig.supplyFan.staticPressure = supply["staticPressure"];
        }

        if (root["exhaustFan"].is<JsonObject>())
        {
            JsonObject exhaust = root["exhaustFan"].as<JsonObject>();
            if (exhaust["minRPM"].is<uint32_t>())
                fansConfig.exhaustFan.minRPM = exhaust["minRPM"];
            if (exhaust["maxRPM"].is<uint32_t>())
                fansConfig.exhaustFan.maxRPM = exhaust["maxRPM"];
            if (exhaust["minPWM"].is<uint32_t>())
                fansConfig.exhaustFan.minPWM = exhaust["minPWM"];
            if (exhaust["airflow"].is<float>())
                fansConfig.exhaustFan.airflow = exhaust["airflow"];
            if (exhaust["staticPressure"].is<float>())
                fansConfig.exhaustFan.staticPressure = exhaust["staticPressure"];
        }

        ESP_LOGV(FansConfig::TAG, "Fans configuration updated.");

        return StateUpdateResult::CHANGED;
    }
};

class FansConfigService : public StatefulService<FansConfig>
{
public:
    FansConfigService(ESP32SvelteKit *sveltekit);

    void begin();

private:
    HttpEndpoint<FansConfig> _httpEndpoint;
    FSPersistence<FansConfig> _fsPersistence;
};
