#include <FanController.h>
#include <OneWireESP32.h>
#include <driver/ledc.h>
#include <ControllerSettingsService.h>

void FanController::_ctrlLoop()
{
    controller_settings_t ctrl_settings;
    uint32_t targetDutyCycle = 0;

    ESP_LOGI(pcTaskGetName(0), "Started.");

    while (1)
    {
        vTaskDelay(CONTROLLER_INTERVALL_MS / portTICK_PERIOD_MS);

        /* Get current controller settings */
        esp_err_t res = _controllerSettingsService.getCurrentSettings(&ctrl_settings);
        if (res != ESP_OK)
        {
            ESP_LOGE(pcTaskGetName(0), "Failed to get current controller settings: %s", esp_err_to_name(res));
            continue;
        }

        /* Check controller prerequisites */
        if (0 == ctrl_settings.tempSensorAddr)
        {
            ESP_LOGE(pcTaskGetName(0), "Relevant temperature sensor address is not yet set.");
            continue;
        }
        if (!_tempSensorsService.isSensorOnline(ctrl_settings.tempSensorAddr))
        {
            ESP_LOGE(pcTaskGetName(0), "Relevant temperature sensor 0x%llx is no longer available. Please set a new one.", ctrl_settings.tempSensorAddr);
            continue;
        }

        /* Get relevant temperature */
        float temp;
        res = _tempSensorsService.getTemperature(ctrl_settings.tempSensorAddr, temp);
        if (res != ESP_OK)
        {
            ESP_LOGE(pcTaskGetName(0), "Failed to get temperature of sensor 0x%llx: %s", ctrl_settings.tempSensorAddr, esp_err_to_name(res));
            continue;
        }

        ESP_LOGV(pcTaskGetName(0), "Current controller settings: lowerTemp=%lu°C, upperTemp=%lu°C, minDutyCycle=%lu%%, maxDutyCycle=%lu%%, tempSensorAddr=0x%llx (%s)",
                 ctrl_settings.lowerTemp, ctrl_settings.upperTemp,
                 ctrl_settings.minDutyCycle, ctrl_settings.maxDutyCycle,
                 ctrl_settings.tempSensorAddr, _tempSensorsService.getSensorName(ctrl_settings.tempSensorAddr).c_str());

        /* Calculate new duty cycle */
        if (temp < ctrl_settings.lowerTemp)
        {
            targetDutyCycle = ctrl_settings.minDutyCycle;
        }
        else if (temp > ctrl_settings.upperTemp)
        {
            targetDutyCycle = ctrl_settings.maxDutyCycle;
        }
        else
        {
            targetDutyCycle = ((temp - ctrl_settings.lowerTemp) /
                               static_cast<float>(ctrl_settings.upperTemp - ctrl_settings.lowerTemp)) *
                                  (ctrl_settings.maxDutyCycle - ctrl_settings.minDutyCycle) +
                              ctrl_settings.minDutyCycle;
        }

        ESP_LOGV(pcTaskGetName(0), "New target duty cycle for %.1f °C: %lu%%", temp, targetDutyCycle);

        /* Update duty cycle */
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, targetDutyCycle * MAX_DUTY_CYCLE / 100);
        if (err != ESP_OK)
        {
            ESP_LOGE(pcTaskGetName(0), "Failed to set duty cycle: %s", esp_err_to_name(err));
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        if (err != ESP_OK)
        {
            ESP_LOGE(pcTaskGetName(0), "Failed to update duty cycle: %s", esp_err_to_name(err));
        }

        /* Publish status to frontend */
        _emitStatus();
    }
}

FanController::FanController(ESP32SvelteKit *sveltekit) : _server(sveltekit->getServer()),
                                                          _securityManager(sveltekit->getSecurityManager()),
                                                          _eventSocket(sveltekit->getSocket()),
                                                          _controllerSettingsService(sveltekit),
                                                          _tempSensorsService(sveltekit, 2), // GPIO 2 for 1-wire bus
                                                          _txTaskHandle(NULL)
{
}

void FanController::begin()
{
    _tempSensorsService.begin();
    _controllerSettingsService.begin();

    _eventSocket->registerEvent(CONTROLLER_STATE_EVENT_ID);

    /* Start the controller task */
    BaseType_t xReturned = xTaskCreatePinnedToCore(
        this->_ctrlLoopImpl,        // Function that should be called
        "fan-ctrl",                 // Name of the task (for debugging)
        4096,                       // Stack size (bytes)
        this,                       // Pass reference to this class instance
        (tskIDLE_PRIORITY + 2),     // task priority
        NULL,                       // Task handle
        ESP32SVELTEKIT_RUNNING_CORE // Pin to application core
    );

    if (xReturned != pdPASS)
    {
        ESP_LOGE(FanController::TAG, "Temperature acquisition task creation failed.");
        return;
    }

    ESP_LOGI(FanController::TAG, "Temperature acquisition task created: %p", _txTaskHandle);

    /* Set up PWM */
    esp_err_t err;
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 25000,
        .clk_cfg = LEDC_AUTO_CLK};

    err = ledc_timer_config(&timer);
    if (err != ESP_OK)
    {
        ESP_LOGE(FanController::TAG, "Failed to configure PWM timer: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(FanController::TAG, "PWM timer configured @ %d Hz", timer.freq_hz);

    ledc_channel_config_t channel = {
        .gpio_num = 1, // GPIO 1 for PWM output
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 1023, // Full duty cycle (@ 10-bit resolution)
        .hpoint = 0};

    err = ledc_channel_config(&channel);
    if (err != ESP_OK)
    {
        ESP_LOGE(FanController::TAG, "Failed to configure PWM channel: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(FanController::TAG, "PWM channel configured @ GPIO %d", channel.gpio_num);

    /* Register endpoints for CC1101 status */
    _server->on(CONTROLLER_STATUS_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&FanController::_handlerGetStatus, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));
}

esp_err_t FanController::_statusAsJSON(JsonObject &root)
{
    if (!root)
    {
        ESP_LOGE(FanController::TAG, "Invalid JSON object provided.");
        return ESP_ERR_INVALID_ARG;
    }

    root["tempCompressor"] = 0;
    root["tempCondenser"] = 0;
    root["rpmSupplyFan"] = 0;        // Placeholder, replace with actual RPM reading
    root["rpmExhaustFan"] = 0;       // Placeholder, replace with actual RPM reading
    root["dutyCycleSupplyFan"] = 0;  // Placeholder, replace with actual duty cycle
    root["dutyCycleExhaustFan"] = 0; // Placeholder, replace with actual duty cycle

    return ESP_OK;
}

esp_err_t FanController::_handlerGetStatus(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject json = response.getRoot();

    if (_statusAsJSON(json) != ESP_OK)
    {
        ESP_LOGE(FanController::TAG, "Failed to get controller status as JSON.");
        return request->reply(HTTPD_500_INTERNAL_SERVER_ERROR, "text/plain", "Failed to get controller status.");
    }

    return response.send();
}

void FanController::_emitStatus()
{
    JsonDocument jsonDoc;
    JsonObject jsonRoot = jsonDoc.to<JsonObject>();
    _statusAsJSON(jsonRoot);
    _eventSocket->emitEvent(CONTROLLER_STATE_EVENT_ID, jsonRoot);
}