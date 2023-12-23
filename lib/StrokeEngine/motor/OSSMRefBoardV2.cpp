#include <Arduino.h>
#include <motor/OSSMRefBoardV2.h>

void OSSMRefBoardV2Motor::begin(motorProperties *motor, OSSMRefBoardV2Properties *OSSMRefBoardV2)
{
    GenericStepperMotor::begin(motor);
    _OSSMRefBoardV2 = OSSMRefBoardV2;
    pinMode(_OSSMRefBoardV2->alarmPin, INPUT_PULLUP);
    pinMode(_OSSMRefBoardV2->inPositionPin, INPUT_PULLUP);
    analogReadResolution(12);
}

float OSSMRefBoardV2Motor::getCurrent(int samples = 20)
{
    // iterate over the samples and sum up the ADC readings
    int sum = 0;
    for (int i = 0; i < samples; i++)
    {
        sum += analogReadMilliVolts(_OSSMRefBoardV2->ADCPinCurrent) - _OSSMRefBoardV2->AmpereOffsetInmV;
    }
    // calculate the average ADC reading
    float average = float(sum) / float(samples);
    // scale the value
    float current = average * _OSSMRefBoardV2->AmperePermV;

    return current;
}

float OSSMRefBoardV2Motor::getVoltage(int samples = 20)
{
    // iterate over the samples and sum up the ADC readings
    int sum = 0;
    for (int i = 0; i < samples; i++)
    {
        sum += analogReadMilliVolts(_OSSMRefBoardV2->ADCPinVoltage);
    }
    // calculate the average ADC reading
    float average = float(sum) / float(samples);
    // scale the value
    float voltage = average * _OSSMRefBoardV2->VoltPermV;

    return voltage;
}

float OSSMRefBoardV2Motor::getPower(int samples = 20)
{
    // iterate over the samples and sum up the ADC readings for current and voltage
    int sumCurrent = 0;
    int sumVoltage = 0;
    for (int i = 0; i < samples; i++)
    {
        sumCurrent += analogReadMilliVolts(_OSSMRefBoardV2->ADCPinCurrent) - _OSSMRefBoardV2->AmpereOffsetInmV;
        sumVoltage += analogReadMilliVolts(_OSSMRefBoardV2->ADCPinVoltage);
    }
    // calculate the average ADC reading
    float averageCurrent = float(sumCurrent) / float(samples);
    float averageVoltage = float(sumVoltage) / float(samples);
    // subtract the offset and scale the value
    float current = averageCurrent * _OSSMRefBoardV2->AmperePermV;
    float voltage = averageVoltage * _OSSMRefBoardV2->VoltPermV;
    // calculate the power
    float power = current * voltage;

    return power;
}

bool OSSMRefBoardV2Motor::_queryHome(int samples = 20)
{
    // check if a homing switch is set
    if (_OSSMRefBoardV2->inPositionPin >> -1)
    {
        return (digitalRead(_homingPin) == !_homingActiveLow) ? true : false;
    }
    else
    {
        // read current
        float current = getCurrent(samples);
        // check if current is above threshold
        if (current > (_idleCurrent + _OSSMRefBoardV2->currentThreshold))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

void OSSMRefBoardV2Motor::_beginHoming()
{
    // measure idle current
    _idleCurrent = getCurrent();
    ESP_LOGI("OSSMRefBoardV2", "Idle Current: %.2f A", _idleCurrent);
}

void OSSMRefBoardV2Motor::measureRailLength(float keepout = 5.0)
{
    // Quit if stepper not enabled
    if (_enabled == false)
    {
        ESP_LOGE("OSSMRefBoardV2", "Measuring not possible! --> Enable stepper first!");
        return;
    }

    // Check if the current sensor is used for homing
    if (_OSSMRefBoardV2->ADCPinCurrent >> -1)
    {
        ESP_LOGE("OSSMRefBoardV2", "Measuring not possible! --> Needs current sensor!");
        return;
    }

    // first stop current motion and suspend motion tasks
    stopMotion();

    // store the keepout distance
    _keepout = keepout;

    ESP_LOGI("OSSMRefBoardV2", "Measuring rail length...");

    // create a one-time task to perform the measure procedure
    xTaskCreatePinnedToCore(
        this->_measureProcedureImpl, // Function that should be called
        "Measuring",                 // Name of the task (for debugging)
        4096,                        // Stack size (bytes)
        this,                        // Pass reference to this class instance
        1,                           // Pretty high task priority
        &_taskMeasuringHandle,       // Task handle
        1                            // Have it on application core
    );
    ESP_LOGD("OSSMRefBoardV2", "Created Homing Task.");
}

void OSSMRefBoardV2Motor::_measureProcedure()
{
    // home the motor
    home();

    // wait until the motor is in position
    while (!_homed)
    {
        // Pause the task for 20ms to allow other tasks
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // measure the rail length
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration);
    // move motor into the other direction
    _stepper->move(_motor->stepsPerMillimeter * (_maxPosition + 4 * _keepout));
    // wait until the motor is in position
    while (!_motionCompleted)
    {
        // query endstop
        if (_queryHome())
        {
            // stop the motor
            _stepper->stopMove();

            // set the current position as the travel length. Keepout already added on one side by homing.
            setMachineGeometry(_stepper->getCurrentPosition() + _keepout, _keepout);

            // move the motor back to the home position
            _stepper->move(_motor->stepsPerMillimeter * (_maxPosition - _keepout));
        }

        // Pause the task for 20ms to allow other tasks
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // delete one-time task
    _taskMeasuringHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("OSSMRefBoardV2", "Measuring task self-terminated");
}

void OSSMRefBoardV2Motor::_callBackMotionPoint()
{
    // Return results of current motion point via the callback
    _cbMotionPoint(
        millis(),
        getPosition(),
        getSpeed(),
        getVoltage(),
        getCurrent());
}
