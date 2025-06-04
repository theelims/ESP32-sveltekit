#include <RPMSensor.h>

RPMSensor::RPMSensor(ESP32SvelteKit *sveltekit, uint8_t pinSupplyFan, uint8_t pinExhaustFan) : _sveltekit(sveltekit),
                                                                                               _eventSocket(sveltekit->getSocket()),
                                                                                               _accessMutex(xSemaphoreCreateRecursiveMutex()),
                                                                                               _pinSupplyFan(pinSupplyFan),
                                                                                               _pinExhaustFan(pinExhaustFan),
                                                                                               _lastLooped(0),
                                                                                               _pcnt_unit(NULL),
                                                                                               _pcnt_chan_a(NULL),
                                                                                               _lastAcquired(0),
                                                                                               _rpm_supplyFan(0),
                                                                                               _rpm_exhaustFan(0)
{
}

void RPMSensor::begin()
{
    /* Register WS event */
    _eventSocket->registerEvent(RPM_SENSORS_EVENT_ID);

    /* --- Init pulse counter--- */
    /* Install PCNT unit */
    pcnt_unit_config_t unit_config = {
        .low_limit = SHRT_MIN,
        .high_limit = SHRT_MAX};
    esp_err_t res = pcnt_new_unit(&unit_config, &_pcnt_unit);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create PCNT unit: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Installed Pulse Counter (PCNT) unit.");

    /* Set glitch filter for PCNT unit */
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000};
    res = pcnt_unit_set_glitch_filter(_pcnt_unit, &filter_config);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set glitch filter: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Set glitch filter to %d ns.", RPM_SENSOR_GLITCH_NS);

    /* Create PCNT channel A for supply fan */
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = _pinSupplyFan,
        .level_gpio_num = -1, // No level signal necessary for this channel
        .flags = {
            .virt_level_io_level = 1 // Set virtual level IO level to high
        }};
    pcnt_channel_handle_t _pcnt_chan_a = NULL;
    res = pcnt_new_channel(_pcnt_unit, &chan_a_config, &_pcnt_chan_a);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create PCNT channel A: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Installed PCNT channel A.");

    /* Set edge actions */
    res = pcnt_channel_set_edge_action(_pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set edge action for PCNT channel(s): %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Set edge actions for PCNT channel(s).");

    /* Enbale PCNT unit. */
    res = pcnt_unit_enable(_pcnt_unit);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable PCNT unit: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Enabled PCNT unit.");

    /* Clear PCNT counter value */
    res = pcnt_unit_clear_count(_pcnt_unit);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to clear PCNT counter value: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGV(TAG, "Cleared PCNT counter value.");

    /* Start PCNT unit */
    res = pcnt_unit_start(_pcnt_unit);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start PCNT unit: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGI(TAG, "Started PCNT unit.");

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

void RPMSensor::_acquireRPMs()
{
    int pulse_count = 0;
    esp_err_t res = pcnt_unit_get_count(_pcnt_unit, &pulse_count);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get PCNT count: %s", esp_err_to_name(res));
        return;
    }

    if (pulse_count < 0)
    {
        ESP_LOGE(TAG, "Negative pulse count detected: %d", pulse_count);
        _rpm_supplyFan = 0;
        return;
    }

    uint32_t currentTime_ms = millis();
    if (_lastAcquired > 0)
    {
        uint32_t timeElapsed_ms = currentTime_ms - _lastAcquired;
        if (timeElapsed_ms > 0)
        {
            _rpm_supplyFan = ((pulse_count * 60000) / timeElapsed_ms / 2); // two pulses per revolution
            //ESP_LOGV(TAG, "Acquired supply fan RPM: %d min-1 (pulse count: %d, time elapsed: %d ms)", _rpm_supplyFan, pulse_count, timeElapsed_ms);
        }
    }

    pcnt_unit_clear_count(_pcnt_unit);
    _lastAcquired = currentTime_ms;
}

void RPMSensor::_emitSensorValues()
{
    JsonDocument jsonDoc;
    JsonObject jsonRoot = jsonDoc.to<JsonObject>();

    _beginTransaction();
    jsonRoot["supplyFan"]["rpm"] = _rpm_supplyFan;
    jsonRoot["exhaustFan"]["rpm"] = _rpm_exhaustFan;
    _endTransaction();

    _eventSocket->emitEvent(RPM_SENSORS_EVENT_ID, jsonRoot);
}