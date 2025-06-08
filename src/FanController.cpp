#include <FanController.h>
#include <OneWireESP32.h>
#include <driver/ledc.h>
#include <ControllerSettingsService.h>

FanController::FanController(ESP32SvelteKit *sveltekit) : _sveltekit(sveltekit),
                                                          _server(sveltekit->getServer()),
                                                          _securityManager(sveltekit->getSecurityManager()),
                                                          _eventSocket(sveltekit->getSocket()),
                                                          _controllerSettingsService(sveltekit),
                                                          _alarmService(sveltekit),
                                                          _fansConfigService(sveltekit),
                                                          _tempSensorsService(sveltekit, &_alarmService, 2), // GPIO 2 for 1-wire bus
                                                          _rpmSensor(sveltekit, &_alarmService, 8, 7),       // GPIO 8 for supply fan, GPIO 7 for exhaust fan
                                                          _accessMutex(xSemaphoreCreateRecursiveMutex()),
                                                          _lastAcquired(0)
{
}

void FanController::begin()
{
    _fansConfigService.begin();
    _tempSensorsService.begin();
    _rpmSensor.begin();
    _controllerSettingsService.begin();
    _alarmService.begin();

    _eventSocket->registerEvent(CONTROLLER_STATE_EVENT_ID);

    /* Enable acquisition loop */
    _sveltekit->addLoopFunction(std::bind(&FanController::loop, this));

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

    /* Register endpoint to get controller status */
    _server->on(CONTROLLER_STATE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&FanController::_handlerGetState, this, std::placeholders::_1),
                                              AuthenticationPredicates::NONE_REQUIRED));
}

void FanController::loop()
{
    uint32_t currentMillis = millis();
    uint32_t timeElapsed = currentMillis - _lastAcquired;
    if (timeElapsed >= CONTROLLER_INTERVALL_MS)
    {
        _lastAcquired = currentMillis;

        controller_settings_t ctrl_settings;
        uint32_t targetDutyCycle = 100;
        float temp = 0.0f;
        bool calculationSuccessful = false;

        /* Get current controller settings */
        esp_err_t res = _controllerSettingsService.getCurrentSettings(&ctrl_settings);
        if (res != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to get current controller settings: %s", esp_err_to_name(res));
        }
        else
        {
            /* Check controller prerequisites */
            if (0 == ctrl_settings.tempSensorAddr)
            {
                ESP_LOGE(TAG, "Relevant temperature sensor address is not yet set.");
            }
            else
            {
                if (!_tempSensorsService.isSensorOnline(ctrl_settings.tempSensorAddr))
                {
                    ESP_LOGE(TAG, "Relevant temperature sensor 0x%llx is no longer available. Please set a new one.",
                             ctrl_settings.tempSensorAddr);
                }
                else
                {
                    /* Get temperature for controller */
                    res = _tempSensorsService.getTemperature(ctrl_settings.tempSensorAddr, temp);
                    if (res != ESP_OK)
                    {
                        ESP_LOGE(TAG, "Failed to get temperature of sensor 0x%llx (%s)",
                                 ctrl_settings.tempSensorAddr,
                                 esp_err_to_name(res));
                    }
                }

                /* Calculate new duty cycle */
                if (temp <= ctrl_settings.lowerTemp)
                {
                    targetDutyCycle = ctrl_settings.minDutyCycle;
                }
                else if (temp >= ctrl_settings.upperTemp)
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

                calculationSuccessful = true;
                ESP_LOGV(TAG, "New target duty cycle for %.1f °C: %lu%%", temp, targetDutyCycle);
            }
        }

        if (!calculationSuccessful)
            ESP_LOGE(TAG, "Controller calculation failed. Using default duty cycle of 100%%.");

        /* Update duty cycle */
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, targetDutyCycle * MAX_DUTY_CYCLE / 100);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to set duty cycle: %s", esp_err_to_name(err));
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to update duty cycle: %s", esp_err_to_name(err));
        }

        /* Obtain full state */
        _beginTransaction();
        _state.baseTemp = temp;
        _state.dutyCycle = targetDutyCycle;
        _state.fan1RPM = _rpmSensor.getRPMSupplyFan();
        _state.fan2RPM = _rpmSensor.getRPMExhaustFan();
        _endTransaction();

        /* Publish status to frontend */
        _emitState();
    }
}

esp_err_t FanController::_stateAsJSON(JsonObject &root)
{
    if (!root)
    {
        ESP_LOGE(FanController::TAG, "Invalid JSON object provided.");
        return ESP_ERR_INVALID_ARG;
    }

    _beginTransaction();
    root["baseTemp"] = _state.baseTemp;
    root["dutyCycle"] = _state.dutyCycle;
    root["fan1RPM"] = _state.fan1RPM;
    root["fan2RPM"] = _state.fan2RPM;
    esp_err_t res = _tempSensorsService.temperaturesAsJson(root);
    _endTransaction();

    return res;
}

esp_err_t FanController::_handlerGetState(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject json = response.getRoot();

    if (_stateAsJSON(json) != ESP_OK)
    {
        ESP_LOGE(FanController::TAG, "Failed to get controller status as JSON.");
        return request->reply(HTTPD_500_INTERNAL_SERVER_ERROR, "text/plain", "Failed to get controller status.");
    }

    return response.send();
}

void FanController::_emitState()
{
    JsonDocument jsonDoc;
    JsonObject jsonRoot = jsonDoc.to<JsonObject>();
    _stateAsJSON(jsonRoot);
    _eventSocket->emitEvent(CONTROLLER_STATE_EVENT_ID, jsonRoot);
}