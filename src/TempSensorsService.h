#pragma once

#include <ESP32SvelteKit.h>
#include <OneWireESP32.h>

#define TEMP_SENSORS_PATH "/rest/sensors"
#define TEMP_SENSORS_FILE "/config/sensors.json"

#define TEMP_SENSORS_UPDATE_FROM_DISCOVERY "update-by-discovery"
#define TEMP_SENSORS_ACQUISITION_INTERVAL_MS 1000 // 1 second
#define TEMP_SENSORS_MAX_ACQUISITION_DURATION_MS 750 // 750 milliseconds

#define TEMP_SENSORS_EVENT_ID "tempsensors"

typedef struct temp_sensor_
{
    uint64_t address;  // 64-bit address of the sensor
    bool online;       // Is the sensor online?
    String name;       // Name of the sensor
} temp_sensor_t;

class TempSensors
{
public:
    static constexpr const char *TAG = "TempSensors";

    std::vector<temp_sensor_t> sensors;

    static void read(TempSensors &tempSsensors, JsonObject &root)
    {
        JsonArray jsonSensors = root["sensors"].to<JsonArray>();
        for (auto &sensor : tempSsensors.sensors)
        {
            JsonObject jsonSensor = jsonSensors.add<JsonObject>();
            jsonSensor["address"] = String(sensor.address);
            jsonSensor["online"] = sensor.online;
            jsonSensor["name"] = sensor.name;
        }

        ESP_LOGV(TempSensors::TAG, "Temperature sensors read.");
    }

    static StateUpdateResult update(JsonObject &root, TempSensors &tempSsensors)
    {
        tempSsensors.sensors.clear();
        if (root["sensors"].is<JsonArray>()) {
            JsonArray jsonSensors = root["sensors"].as<JsonArray>();
            for (JsonObject jsonSensor : jsonSensors)
            {
                temp_sensor_t sensor;
                sensor.address = strtoull(jsonSensor["address"].as<String>().c_str(), nullptr, 10);
                sensor.online = jsonSensor["online"].as<bool>() | false;
                sensor.name = jsonSensor["name"].as<String>();

                tempSsensors.sensors.push_back(sensor);
            }
        }
        
        ESP_LOGV(TempSensors::TAG, "Temperature sensors updated.");
        return StateUpdateResult::CHANGED;
    }
};

class TempSensorsService : public StatefulService<TempSensors>
{
public:
    static constexpr const char *TAG = "TempSensorsService";

    TempSensorsService(ESP32SvelteKit *sveltekit, uint8_t bus_pin = 2);

    void begin();
    void _loop();

    bool isSensorOnline(uint64_t &address);
    esp_err_t getTemperature(uint64_t &address, float &temperature);
    String getSensorName(uint64_t &address);

private:
    static const uint8_t MAX_NUM_DEVS = 5; // Maximum number of devices on the bus    

    ESP32SvelteKit *_sveltekit;
    HttpEndpoint<TempSensors> _httpEndpoint;
    FSPersistence<TempSensors> _fsPersistence;
    EventSocket *_eventSocket;

    

    OneWire32 _ds_bus;
    std::map<uint64_t, float> _temperatures;
    volatile uint32_t _lastAcquired; // Last time (millies) the temperatures were acquired

    void _updateSensors();
    void _acquireTemps();
    bool _isSensorKnown(uint64_t &address, uint32_t &index);
    void _emitSensorValues();
};
