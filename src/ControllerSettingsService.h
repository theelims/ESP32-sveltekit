#pragma once

#include <ESP32SvelteKit.h>

#define CONTROLLER_SETTINGS_PATH "/rest/controller/settings"
#define CONTROLLER_SETTINGS_FILE "/config/controller-settings.json"

typedef struct controller_settings_
{
    uint32_t lowerTemp;     // °C
    uint32_t upperTemp;     // °C
    uint32_t minDutyCycle;  // %
    uint32_t maxDutyCycle;  // %
    uint64_t tempSensorAddr; // 64-bit address of the relevant temperature sensor

} controller_settings_t;

class ControllerSettings
{

public:
    static constexpr const char *TAG = "ControllerSettings";

    ControllerSettings()
    {
        main = {
            .lowerTemp = 20,
            .upperTemp = 35,
            .minDutyCycle = 0,
            .maxDutyCycle = 100,
            .tempSensorAddr = 0      // Address not yet set
        };
    }

    controller_settings_t main;

    static void read(ControllerSettings &settings, JsonObject &root)
    {
        root["lowerTemp"] = settings.main.lowerTemp;
        root["upperTemp"] = settings.main.upperTemp;
        root["minDutyCycle"] = settings.main.minDutyCycle;
        root["maxDutyCycle"] = settings.main.maxDutyCycle;
        root["tempSensorAddr"] = String(settings.main.tempSensorAddr);

        ESP_LOGV(ControllerSettings::TAG, "Fan controller settings read.");
    }

    static StateUpdateResult update(JsonObject &root, ControllerSettings &settings)
    {
        uint32_t newValue;
        bool changed = false;

        /* Lower Temperature */
        if (root["lowerTemp"].is<uint32_t>())
        {
            newValue = root["lowerTemp"];
            if (newValue != settings.main.lowerTemp)
            {
                settings.main.lowerTemp = newValue;
                changed |= true;
            }
        }

        /* Upper Temperature */
        if (root["upperTemp"].is<uint32_t>())
        {
            newValue = root["upperTemp"];
            if (newValue != settings.main.upperTemp)
            {
                settings.main.upperTemp = newValue;
                changed |= true;
            }
        }

        /* Minimum Duty Cycle */
        if (root["minDutyCycle"].is<uint32_t>())
        {
            newValue = root["minDutyCycle"];
            if (newValue != settings.main.minDutyCycle)
            {
                settings.main.minDutyCycle = newValue;
                changed |= true;
            }
        }

        /* Maximum Duty Cycle */
        if (root["maxDutyCycle"].is<uint32_t>())
        {
            newValue = root["maxDutyCycle"];
            if (newValue != settings.main.maxDutyCycle)
            {
                settings.main.maxDutyCycle = newValue;
                changed |= true;
            }
        }

        /* Relevant Sensor */
        if (root["tempSensorAddr"].is<String>())
        {
            uint64_t newValue64 = strtoull(root["tempSensorAddr"].as<String>().c_str(), nullptr, 10);
            if (newValue64 != settings.main.tempSensorAddr)
            {
                settings.main.tempSensorAddr = newValue64;
                changed |= true;
            }
        }

        if (changed)
            ESP_LOGI(ControllerSettings::TAG, "Fan controller settings updated.");
        else
            ESP_LOGI(ControllerSettings::TAG, "Fan controller settings unchanged.");

        return changed ? StateUpdateResult::CHANGED : StateUpdateResult::UNCHANGED;
    }
};

class ControllerSettingsService : StatefulService<ControllerSettings>
{
public:
    static constexpr const char *TAG = "TempSensors";

    ControllerSettingsService(ESP32SvelteKit *sveltekit);

    void begin();

    esp_err_t getCurrentSettings(controller_settings_t *settings);

private:
    HttpEndpoint<ControllerSettings> _httpEndpoint;
    FSPersistence<ControllerSettings> _fsPersistence;
};