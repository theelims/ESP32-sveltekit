#include <TempSensorsService.h>
#include <OneWireESP32.h>

const char *errt[] = {"", "CRC", "BAD", "DC", "DRV"};

TempSensorsService::TempSensorsService(ESP32SvelteKit *sveltekit, AlarmService *alarmService, uint8_t bus_pin) : _sveltekit(sveltekit),
                                                                                                                 _httpEndpoint(TempSensors::read,
                                                                                                                               TempSensors::update,
                                                                                                                               this,
                                                                                                                               sveltekit->getServer(),
                                                                                                                               TEMP_SENSORS_PATH,
                                                                                                                               sveltekit->getSecurityManager(),
                                                                                                                               AuthenticationPredicates::IS_ADMIN),

                                                                                                                 _fsPersistence(TempSensors::read,
                                                                                                                                TempSensors::update,
                                                                                                                                this,
                                                                                                                                sveltekit->getFS(),
                                                                                                                                TEMP_SENSORS_FILE),
                                                                                                                 _eventSocket(sveltekit->getSocket()),
                                                                                                                 _alarmService(alarmService),
                                                                                                                 _lastAcquired(0),
                                                                                                                 _ds_bus(bus_pin)
{
}

void TempSensorsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();

    _eventSocket->registerEvent(TEMP_VALUES_EVENT_ID);
    _eventSocket->registerEvent(TEMP_SENSORS_EVENT_ID);

    /* Discover current temperature sensors */
    _updateSensors();

    /* Enable acquisition loop */
    _sveltekit->addLoopFunction(std::bind(&TempSensorsService::loop, this));
}

bool TempSensorsService::isSensorOnline(uint64_t &address)
{
    bool online = false;

    beginTransaction();
    for (const auto &sensor : _state.sensors)
    {
        if (sensor.address == address)
        {
            online = sensor.online;
            break;
        }
    }
    endTransaction();

    return online;
}

esp_err_t TempSensorsService::getTemperature(uint64_t &address, float &temperature)
{
    if (_temperatures.find(address) == _temperatures.end())
    {
        ESP_LOGE(TempSensorsService::TAG, "No temperature value for sensor with address 0x%llx found. Did the sensor went offline?", address);
        return ESP_ERR_NOT_FOUND;
    }

    beginTransaction();
    temperature = _temperatures[address];
    endTransaction();

    return ESP_OK;
}

String TempSensorsService::getSensorName(uint64_t &address)
{
    uint32_t sensorIdx;
    if (!_isSensorKnown(address, sensorIdx))
    {
        return String();
    }

    beginTransaction();
    String name = _state.sensors[sensorIdx].name;
    endTransaction();

    return name;
}

void TempSensorsService::_updateSensors()
{
    uint64_t addr[MAX_NUM_DEVS];

    ESP_LOGI(TempSensorsService::TAG, "Searching 1-wire devices...");

    beginTransaction();

    uint8_t num_devs = _ds_bus.search(addr, MAX_NUM_DEVS);

    /* Check existing (stored) sensors */
    for (auto &sensor : _state.sensors)
    {
        sensor.online = false; // Assume sensor is offline
        for (uint8_t i = 0; i < num_devs; i++)
        {
            if (sensor.address == addr[i])
            {
                sensor.online = true;
                addr[i] = 0; // Mark this address as found
                break;
            }
        }
    }

    /* Add new sensors */
    for (uint8_t i = 0; i < num_devs; i++)
    {
        if (addr[i] != 0)
        { // Skip marked addresses
            temp_sensor_t new_sensor;
            new_sensor.address = addr[i];
            new_sensor.online = true;
            new_sensor.name = "";
            _state.sensors.push_back(new_sensor);
            ESP_LOGI(TempSensorsService::TAG, "New sensor added: 0x%llx", addr[i]);
        }
    }

    endTransaction();

    callUpdateHandlers(TEMP_SENSORS_UPDATE_FROM_DISCOVERY);
}

void TempSensorsService::loop()
{
    uint32_t currentMillis = millis();
    uint32_t timeElapsed = currentMillis - _lastAcquired;
    if (timeElapsed >= max(TEMP_SENSORS_ACQUISITION_INTERVAL_MS, TEMP_SENSORS_MAX_ACQUISITION_DURATION_MS))
    {
        _lastAcquired = currentMillis;

        /* Acquire temperatures from all sensors */
        _acquireTemps();

        /* Emit sensor values */
        _emitSensorValues();
    }
}

void TempSensorsService::_acquireTemps()
{
    /* Request simultaneous temperature readings from all sensors */
    _ds_bus.request();

    vTaskDelay(TEMP_SENSORS_MAX_ACQUISITION_DURATION_MS / portTICK_PERIOD_MS); // Max. conversion of DS18B20 for 12 bit resolution is 750 ms

    bool onlineChanged = false;

    beginTransaction();
    for (auto &sensor : _state.sensors)
    {
        uint8_t res = _ds_bus.getTemp(sensor.address, _temperatures[sensor.address]);
        if (res != OWR_OK)
        {
            if (sensor.online)
            {
                sensor.readErrors++;
                _temperatures.erase(sensor.address);
                if (sensor.readErrors > TEMP_SENSORS_MAX_READ_ERRORS)
                {
                    sensor.online = false;
                    onlineChanged |= true;
                    ESP_LOGE(TempSensorsService::TAG, "Sensor 0x%llx marked as offline due to too many read errors (>%d).", sensor.address, TEMP_SENSORS_MAX_READ_ERRORS);
                    _alarmService->publishAlarm("Temperature sensor 0x" + String(sensor.address, HEX) + " (" + sensor.name + ") went offline.");
                }
                else
                {
                    ESP_LOGE(TempSensorsService::TAG, "Error reading sensor 0x%llx: %s (read errors: %lu)", sensor.address, errt[res], sensor.readErrors);
                }
            }
        }
        else
        {
            if (!sensor.online)
            {
                sensor.online = true;
                onlineChanged |= true;
                ESP_LOGI(TempSensorsService::TAG, "Sensor 0x%llx is back online after successful read.", sensor.address);
            }

            sensor.readErrors = 0; // Reset read errors if reading was successful

            ESP_LOGV(TempSensorsService::TAG, "Acquired sensor 0x%llx temperature: %.2f °C", sensor.address, _temperatures[sensor.address]);
        }
    }
    endTransaction();

    if (onlineChanged)
    {
        JsonObject emptyJson;
        _eventSocket->emitEvent(TEMP_SENSORS_EVENT_ID, emptyJson);
    }
}

bool TempSensorsService::_isSensorKnown(uint64_t &address, uint32_t &index)
{
    bool found = false;
    beginTransaction();

    for (uint32_t i = 0; i < _state.sensors.size(); i++)
    {
        if (_state.sensors[i].address == address)
        {
            index = i;
            found = true;
            break;
        }
    }
    endTransaction();

    return found;
}

void TempSensorsService::_emitSensorValues()
{
    JsonDocument jsonDoc;
    JsonObject jsonRoot = jsonDoc.to<JsonObject>();
    JsonArray jsonSensors = jsonRoot["temperatures"].to<JsonArray>();

    beginTransaction();
    for (const auto &entry : _temperatures)
    {
        JsonObject sensorObj = jsonSensors.add<JsonObject>();
        sensorObj["address"] = String(entry.first);
        sensorObj["temperature"] = entry.second;
    }
    endTransaction();

    _eventSocket->emitEvent(TEMP_VALUES_EVENT_ID, jsonRoot);
}
