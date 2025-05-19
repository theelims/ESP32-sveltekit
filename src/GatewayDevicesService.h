#ifndef GatewayDevicesService_h
#define GatewayDevicesService_h

#include <EventSocket.h>
#include <FSPersistence.h>
#include <HttpEndpoint.h>
#include <JsonUtils.h>
#include <SecurityManager.h>
#include <PsychicHttp.h>
#include <ESP32SvelteKit.h>
#include <Utils.hpp>

#define GATEWAY_DEVICES_FILE "/config/gateway-devices.json"
#define GATEWAY_DEVICES_SERVICE_PATH "/rest/gateway-devices"

#define GATEWAY_MAX_DEVICES 50
#define GATEWAY_MAX_ALARMS 100

#define GATEWAY_ORIGIN_ID "devices"

typedef enum hekatron_alarm_ending
{
    HAE_MIN = -2,              // Minimum value (for enum range checks)
    HAE_ALARM_ACTIVE = -1,     // Alarm is currently active
    HAE_BY_SMOKE_DETECTOR = 0, // Alarm was ended by smoke detector
    HAE_BY_MANUAL_RESET,       // Alarm was ended by manual reset
    HAE_MAX                    // Maximum value (for enum range checks)
} hekatron_alarm_ending_t;

typedef struct hekatron_device_alarm
{
    time_t startTime;
    time_t endTime;
    hekatron_alarm_ending_t endingReason;
} hekatron_device_alarm_t;

typedef enum hekatron_smoke_detector
{
    HSD_GENIUS_PLUS_X = 0
} HekatronSmokeDetector;

typedef enum hekatron_radio_module
{
    HRM_FM_BASIS_X = 0
} HekatronRadioModule;

/**
 * @brief Template class for Hekatron components
 * @tparam T The type of the model attribute
 */
template <typename T>
class HekatronComponent
{
public:
    HekatronComponent(const T &model,
                      uint32_t sn,
                      time_t productionDate) : model(model),
                                               sn(sn),
                                               productionDate(productionDate)
    {
    }

    void toJson(JsonObject &root)
    {
        root["model"] = model;
        root["sn"] = sn;
        root["productionDate"] = Utils::time_t_to_iso8601(productionDate);
    }

    T model;
    uint32_t sn;
    time_t productionDate; // Production date in seconds since Unix Epoch (UTC)
};

/**
 * @brief Class for Hekatron devices
 */
class HekatronDevice
{
public:
    HekatronDevice(const HekatronComponent<HekatronSmokeDetector> &smokeDetector,
                   const HekatronComponent<HekatronRadioModule> &radioModule,
                   const String &location) : smokeDetector(smokeDetector),
                                             radioModule(radioModule),
                                             location(location)
    {
    }

    void toJson(JsonObject &root)
    {
        // Smoke detector
        JsonObject smokeDetector = root["smokeDetector"].to<JsonObject>();
        this->smokeDetector.toJson(smokeDetector);
        // Radio module
        JsonObject radioModule = root["radioModule"].to<JsonObject>();
        this->radioModule.toJson(radioModule);
        // Location
        root["location"] = this->location;
        // Alarms
        JsonArray alarms = root["alarms"].to<JsonArray>();
        for (auto &alarm : this->alarms)
        {
            JsonObject alarm_as_json = alarms.add<JsonObject>();

            alarm_as_json["startTime"] = Utils::time_t_to_iso8601(alarm.startTime);
            alarm_as_json["endTime"] = Utils::time_t_to_iso8601(alarm.endTime);
            alarm_as_json["endingReason"] = alarm.endingReason;
        }
    }

    HekatronComponent<HekatronSmokeDetector> smokeDetector;
    HekatronComponent<HekatronRadioModule> radioModule;
    String location;
    std::vector<hekatron_device_alarm_t> alarms;
    bool isAlarming = false;
};

class HekatronDevices
{

public:
    static constexpr const char *TAG = "HekatronDevices";

    std::vector<HekatronDevice> devices;

    static void read(HekatronDevices &hekatronDevices, JsonObject &root)
    {
        JsonArray jsonDevices = root["devices"].to<JsonArray>();
        for (auto &device : hekatronDevices.devices)
        {
            JsonObject jsonDevice = jsonDevices.add<JsonObject>();
            device.toJson(jsonDevice);
        }

        ESP_LOGV(HekatronDevices::TAG, "Smoke detector devices configurations read.");
    }

    static StateUpdateResult update(JsonObject &root, HekatronDevices &hekatronDevices)
    {
        if (root["devices"].is<JsonArray>())
        {
            hekatronDevices.devices.clear();

            // iterate over devices
            int i = 0;
            for (JsonVariant jsonDeviceArrItem : root["devices"].as<JsonArray>())
            {
                if (i++ >= GATEWAY_MAX_DEVICES)
                {
                    ESP_LOGE(HekatronDevices::TAG, "Too many smoke detector devices. Maximum allowed is %d.", GATEWAY_MAX_DEVICES);
                    break;
                }

                JsonObject smokeDetector = jsonDeviceArrItem["smokeDetector"].as<JsonObject>();
                JsonObject radioModule = jsonDeviceArrItem["radioModule"].as<JsonObject>();

                HekatronDevice newDevice = HekatronDevice(
                    HekatronComponent<HekatronSmokeDetector>(
                        static_cast<HekatronSmokeDetector>(smokeDetector["model"].as<int>()),
                        smokeDetector["sn"].as<uint32_t>(),
                        Utils::iso8601_to_time_t(smokeDetector["productionDate"].as<String>())),
                    HekatronComponent<HekatronRadioModule>(
                        static_cast<HekatronRadioModule>(radioModule["model"].as<int>()),
                        radioModule["sn"].as<uint32_t>(),
                        Utils::iso8601_to_time_t(radioModule["productionDate"].as<String>())),
                    jsonDeviceArrItem["location"].as<String>());

                if (jsonDeviceArrItem["alarms"].is<JsonArray>())
                {
                    // iterate over alarms
                    int alarms_count = 0;
                    for (JsonVariant jsonAlarm : jsonDeviceArrItem["alarms"].as<JsonArray>())
                    {
                        if (alarms_count++ >= GATEWAY_MAX_ALARMS)
                        {
                            ESP_LOGE(HekatronDevices::TAG, "Too many alarms for a smoke detector devices. Maximum allowed is %d.", GATEWAY_MAX_ALARMS);
                            break;
                        }

                        newDevice.alarms.push_back(hekatron_device_alarm_t{
                            .startTime = Utils::iso8601_to_time_t(jsonAlarm["startTime"].as<String>()),
                            .endTime = Utils::iso8601_to_time_t(jsonAlarm["endTime"].as<String>()),
                            .endingReason = static_cast<hekatron_alarm_ending_t>(jsonAlarm["endingReason"].as<int>())});

                        alarms_count++;
                    }
                }

                hekatronDevices.devices.push_back(newDevice);

                ESP_LOGV(HekatronDevices::TAG, "Added smoke detector with SN '%lu'.", hekatronDevices.devices.back().smokeDetector.sn);

                i++;
            }
        }

        ESP_LOGV(HekatronDevices::TAG, "Smoke detector devices configurations updated.");

        return StateUpdateResult::CHANGED;
    }
};

class GatewayDevicesService : public StatefulService<HekatronDevices>
{
public:
    GatewayDevicesService(ESP32SvelteKit *sveltekit);

    void begin();

    std::vector<HekatronDevice> &getDevices()
    {
        return _state.devices;
    }

    void setAlarm(uint32_t detectorSN);

    void resetAlarm(uint32_t detectorSN, hekatron_alarm_ending_t endingReason);

    bool isAlarming()
    {
        return _isAlarming;
    }

    bool isSmokeDetectorKnown(uint32_t detectorSN)
    {
        bool found = false;
        beginTransaction();
        for (auto &device : _state.devices)
        {
            if (device.smokeDetector.sn == detectorSN)
            {
                found = true;
                break;
            }
        }
        endTransaction();
        return found;
    }

private:
    HttpEndpoint<HekatronDevices> _httpEndpoint;
    FSPersistence<HekatronDevices> _fsPersistence;
    bool _isAlarming = false;

    void updateAlarmState();
};

#endif // GatewayDevicesService_h