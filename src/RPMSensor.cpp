#include <RPMSensor.h>

RPMSensor::RPMSensor(ESP32SvelteKit *sveltekit, AlarmService *alarmService, uint8_t pinSupplyFan, uint8_t pinExhaustFan) : _sveltekit(sveltekit),
                                                                                                                           _eventSocket(sveltekit->getSocket()),
                                                                                                                           _alarmService(alarmService),
                                                                                                                           _accessMutex(xSemaphoreCreateRecursiveMutex()),
                                                                                                                           _pinSupplyFan(pinSupplyFan),
                                                                                                                           _pinExhaustFan(pinExhaustFan),
                                                                                                                           _lastLooped(0),
                                                                                                                           _lastAcquired(0)
{
}

void RPMSensor::begin()
{
    /* Register WS event */
    _eventSocket->registerEvent(RPM_SENSORS_EVENT_ID);

    /* --- Init pulse counters (PCNT) --- */
    /* Install PCNT units */
    pcnt_unit_config_t unit_config = {
        .low_limit = SHRT_MIN,
        .high_limit = SHRT_MAX};
    esp_err_t res = pcnt_new_unit(&unit_config, &_pcnt_units[RPM_INDEX_SUPPLY_FAN]);
    res |= pcnt_new_unit(&unit_config, &_pcnt_units[RPM_INDEX_EXHAUST_FAN]);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create PCNT units: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Installed Pulse Counter (PCNT) units.");

    /* Set glitch filter for PCNT units */
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000 // Ignore pulses shorter than 1000 ns
    };
    res = pcnt_unit_set_glitch_filter(_pcnt_units[RPM_INDEX_SUPPLY_FAN], &filter_config);
    res |= pcnt_unit_set_glitch_filter(_pcnt_units[RPM_INDEX_EXHAUST_FAN], &filter_config);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set glitch filters: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Set glitch filters to %d ns.", RPM_SENSOR_GLITCH_NS);

    /* Create PCNT channels */
    pcnt_chan_config_t chnl_config_fan1 = {
        .edge_gpio_num = _pinSupplyFan,
        .level_gpio_num = -1, // No level signal necessary for this channel
        .flags = {
            .virt_level_io_level = 1 // Set virtual level IO level to high
        }};
    pcnt_chan_config_t chnl_config_fan2 = {
        .edge_gpio_num = _pinExhaustFan,
        .level_gpio_num = -1, // No level signal necessary for this channel
        .flags = {
            .virt_level_io_level = 1 // Set virtual level IO level to high
        }};
    res = pcnt_new_channel(_pcnt_units[RPM_INDEX_SUPPLY_FAN], &chnl_config_fan1, &_pcnt_chnls[RPM_INDEX_SUPPLY_FAN]);
    res |= pcnt_new_channel(_pcnt_units[RPM_INDEX_EXHAUST_FAN], &chnl_config_fan2, &_pcnt_chnls[RPM_INDEX_EXHAUST_FAN]);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create PCNT channels: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Installed PCNT channels.");

    /* Set edge actions */
    res = pcnt_channel_set_edge_action(_pcnt_chnls[RPM_INDEX_SUPPLY_FAN], PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);
    res |= pcnt_channel_set_edge_action(_pcnt_chnls[RPM_INDEX_EXHAUST_FAN], PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set edge action for PCNT channels: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Set edge actions for PCNT channels.");

    /* Enbale PCNT units. */
    res = pcnt_unit_enable(_pcnt_units[RPM_INDEX_SUPPLY_FAN]);
    res |= pcnt_unit_enable(_pcnt_units[RPM_INDEX_EXHAUST_FAN]);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable PCNT units: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Enabled PCNT units.");

    /* Clear PCNT counter value */
    res = pcnt_unit_clear_count(_pcnt_units[RPM_INDEX_SUPPLY_FAN]);
    res |= pcnt_unit_clear_count(_pcnt_units[RPM_INDEX_EXHAUST_FAN]);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to clear PCNT counter values: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Cleared PCNT counter values.");

    /* Start PCNT units */
    res = pcnt_unit_start(_pcnt_units[RPM_INDEX_SUPPLY_FAN]);
    res |= pcnt_unit_start(_pcnt_units[RPM_INDEX_EXHAUST_FAN]);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start PCNT units: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGI(TAG, "Started PCNT units.");

    /* Enable acquisition loop */
    _sveltekit->addLoopFunction(std::bind(&RPMSensor::loop, this));
}

void RPMSensor::loop()
{
    uint32_t currentMillis = millis();
    uint32_t timeElapsed = currentMillis - _lastLooped;
    if (timeElapsed >= RPM_SENSORS_ACQUISITION_INTERVAL_MS)
    {
        _lastLooped = currentMillis;

        /* Acquire RPMs from fans */
        _acquireRPMs();

        /* Emit sensor values */
        _emitSensorValues();
    }
}

// TODO: Not yet perfectly thread-safe
void RPMSensor::_acquireRPMs()
{
    int pulse_count[RPM_NUM_FANS];

    uint32_t currentTime_ms = millis();
    if (0 == _lastAcquired)
    {   _lastAcquired = currentTime_ms;
        return; // Skip first acquisition, as we have no previous timestamp
    }

    uint32_t timeElapsed_ms = currentTime_ms - _lastAcquired;
    if (0 == timeElapsed_ms)
        return; // Skip acquisition if no time has elapsed since last acquisition, to avoid division by zero

    _beginTransaction();
    for (int i = 0; i < RPM_NUM_FANS; i++)
    {
        pulse_count[i] = 0;

        esp_err_t res = pcnt_unit_get_count(_pcnt_units[i], &pulse_count[i]);
        if (res != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to get pulse count for fan 1 (supply fan): %s. Assuming standstill.", esp_err_to_name(res));
            _rpms[i] = 0;
            if (!_errors[i])
            {
                _errors[i] = true;
                _alarmService->publishAlarm("RPM of fan " + String(i) + " could not be determined.");
            }
        }
        else
        {
            _errors[i] = false; // Reset error flag if pulse count was acquired successfully
        }

        if (pulse_count[i] < 0)
        {
            ESP_LOGE(TAG, "Negative pulse count detected for fan %d: %d", i, pulse_count);
            _rpms[i] = 0;
        }
        else
        {
            _rpms[i] = ((pulse_count[i] * 60000) / timeElapsed_ms / 2); // two pulses per revolution
            ESP_LOGV(TAG, "Acquired RPM for fan %d: %d min-1 (pulse count: %d, time elapsed: %d ms)", i, _rpms[i], pulse_count[i], timeElapsed_ms);
        }

        pcnt_unit_clear_count(_pcnt_units[i]);
    }

    _endTransaction();

    _lastAcquired = currentTime_ms;
}

void RPMSensor::_emitSensorValues()
{
    JsonDocument jsonDoc;
    JsonObject jsonRoot = jsonDoc.to<JsonObject>();

    _beginTransaction();
    jsonRoot["supplyFan"]["rpm"] = _rpms[RPM_INDEX_SUPPLY_FAN];
    jsonRoot["exhaustFan"]["rpm"] = _rpms[RPM_INDEX_EXHAUST_FAN];
    _endTransaction();

    _eventSocket->emitEvent(RPM_SENSORS_EVENT_ID, jsonRoot);
}
