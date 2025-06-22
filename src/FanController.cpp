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
                                                          _tempSensorsService(sveltekit, &_alarmService, ONE_WIRE_BUS_GPIO),
                                                          _rpmSensor(sveltekit, &_alarmService, RPM_SUPPLY_FAN_GPIO, RPM_EXHAUST_FAN_GPIO), // GPIO 8 for supply fan, GPIO 7 for exhaust fan
                                                          _accessMutex(xSemaphoreCreateRecursiveMutex()),
                                                          _lastAcquired(0),
                                                          _tempError(false),
                                                          _fanError(false)
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
        .timer_num = CONTROLLER_PWM_TIMER_NUM,
        .freq_hz = 25000,
        .clk_cfg = LEDC_AUTO_CLK};

    err = ledc_timer_config(&timer);
    if (err != ESP_OK)
    {
        ESP_LOGE(FanController::TAG, "Failed to configure PWM timer: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(FanController::TAG, "PWM timer configured @ %d Hz", timer.freq_hz);

    ledc_channel_config_t channel_supply_fan = {
        .gpio_num = PWM_SUPPLY_FAN_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = CONTROLLER_PWM_CHANNEL_SUPPLY_FAN,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = CONTROLLER_PWM_TIMER_NUM,
        .duty = 1023, // Full duty cycle (@ 10-bit resolution)
        .hpoint = 0};

    err = ledc_channel_config(&channel_supply_fan);
    if (err != ESP_OK)
    {
        ESP_LOGE(FanController::TAG, "Failed to configure PWM channel for supply fan: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(FanController::TAG, "PWM channel for supply fan configured @ GPIO %d", channel_supply_fan.gpio_num);

    // Configure second channel for the same PWM signal on another GPIO
    ledc_channel_config_t channel_exhaust_fan = {
        .gpio_num = PWM_EXHAUST_FAN_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = CONTROLLER_PWM_CHANNEL_EXHAUST_FAN,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = CONTROLLER_PWM_TIMER_NUM,
        .duty = 1023, // Full duty cycle (@ 10-bit resolution)
        .hpoint = 0};

    err = ledc_channel_config(&channel_exhaust_fan);
    if (err != ESP_OK)
    {
        ESP_LOGE(FanController::TAG, "Failed to configure PWM channel for exhaust fan: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(FanController::TAG, "PWM channel for exhaust fan configured @ GPIO %d", channel_exhaust_fan.gpio_num);

    /* Configure/set GPIO to enable level shifter */
    gpio_config_t oe_gpio_conf = {
        .pin_bit_mask = (1ULL << LEVEL_SHIFT_IC_OE_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&oe_gpio_conf);

    gpio_set_level((gpio_num_t)LEVEL_SHIFT_IC_OE_GPIO, 1); // Enable level shifter by setting output enable (OE) pin high

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
                    else
                    {
                        /* Check max. temperature */
                        if (ctrl_settings.monitorTemperature && temp > ctrl_settings.maxTemp && !_tempError)
                        {
                            ESP_LOGW(TAG, "Current temperature (%.1f °C) exceeds maximum allowed temperature (%lu °C).",
                                     temp, ctrl_settings.maxTemp);
                            _tempError = true;
                            _alarmService.publishAlarm("Current temperature (" + String(temp, 1) + " °C) exceeds maximum allowed temperature (" + String(ctrl_settings.maxTemp) + " °C).");
                        }
                        else
                        {
                            if (temp < ctrl_settings.maxTemp - CONTROLLER_TEMP_MONITOR_HYSTERESIS) // Reset temperature error if within limits
                                _tempError = false;
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
            }
        }

        if (!calculationSuccessful)
            ESP_LOGE(TAG, "Controller calculation failed. Using default duty cycle of 100%%.");

        /* Update duty cycles */
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, CONTROLLER_PWM_CHANNEL_SUPPLY_FAN, targetDutyCycle * MAX_DUTY_CYCLE / 100);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to set duty cycle of supply fan: %s", esp_err_to_name(err));
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, CONTROLLER_PWM_CHANNEL_SUPPLY_FAN);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to update duty cycle of supply fan: %s", esp_err_to_name(err));
        }

        err = ledc_set_duty(LEDC_LOW_SPEED_MODE, CONTROLLER_PWM_CHANNEL_EXHAUST_FAN, targetDutyCycle * MAX_DUTY_CYCLE / 100);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to set duty cycle of exhaust fan: %s", esp_err_to_name(err));
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, CONTROLLER_PWM_CHANNEL_EXHAUST_FAN);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to update duty cycle of exhaust fan: %s", esp_err_to_name(err));
        }

        /* Check fans */
        uint32_t supplyFanRPM = _rpmSensor.getRPMSupplyFan();
        uint32_t exhaustFanRPM = _rpmSensor.getRPMExhaustFan();
        if (ctrl_settings.monitorFans &&
            targetDutyCycle > 10 &&
            (supplyFanRPM < CONTROLLER_MIN_RPM_SUPPLY_FAN || exhaustFanRPM < CONTROLLER_MIN_RPM_EXHAUST_FAN))
        {
            if (!_fanError)
            {
                ESP_LOGW(TAG, "One or both fans are not running properly. Supply fan RPM: %lu min-1, Exhaust fan RPM: %lu min-1.",
                         supplyFanRPM, exhaustFanRPM);
                _fanError = true;
                _alarmService.publishAlarm("Fan failure detected. Supply fan RPM: " + String(supplyFanRPM) + " min-1, Exhaust fan RPM: " + String(exhaustFanRPM) + " min-1.");
            }
        }
        else
        {
            _fanError = false; // Reset fan error if fans are running properly
        }

        /* Obtain full state */
        _beginTransaction();
        _state.baseTemp = temp;
        _state.dutyCycle = targetDutyCycle;
        _state.fan1RPM = supplyFanRPM;
        _state.fan2RPM = exhaustFanRPM;
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