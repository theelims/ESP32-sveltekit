/**
 *   Motor Driver for the OSSM Reference Board V2 of StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#pragma once

#include <Arduino.h>
#include <FastAccelStepper.h>

#include "motor.h"

/**************************************************************************/
/*!
  @brief  Struct defining the motor (stepper or servo with STEP/DIR
  interface) and the motion system translating the rotation into a
  linear motion.This structure also contains the configuration parameters
  for the OSSMRefBoardV2. It includes pins for alarm, in-position, current
  sensor, voltage sensor, and scaling factors.
*/
/**************************************************************************/
typedef struct
{
  bool enableActiveLow; /*> Polarity of the enable signal. True for active low. */
  int stepPin;          /*> Pin connected to the STEP input */
  int directionPin;     /*> Pin connected to the DIR input */
  int enablePin;        /*> Pin connected to the ENA input */
  int alarmPin;         /*> Pin connected to the ALM input */
  int inPositionPin;    /*> Pin connected to the PED input */
  int ADCPinCurrent;    /*> Pin connected to the current sensor input */
  float AmperePermV;    /*> Scaling of the current sensor ADC */
  int AmpereOffsetInmV; /*> Offset in mV of the current sensor ADC */
  int ADCPinVoltage;    /*> Pin connected to the voltage sensor input */
  float VoltPermV;      /*> Scaling of the voltage ADC */
} OSSMRefBoardV2Properties;

using MeasureCallbackType = std::function<void(float)>;

/**************************************************************************/
/*!
  @brief  OSSMRefBoardV2 Motor inherits from MotorInterface and provides a generic
  STEP/DIR interface to all common stepper and servo drivers. Under the hood
  it uses FastAccelStepper for the trapezoidal motion planning and the
  hardware step signal generation. The generic stepper class uses the built-in
  current sensor for homing.
*/
/**************************************************************************/
class OSSMRefBoardV2Motor : public MotorInterface
{
public:
  /**************************************************************************/
  /*!
   @brief Initializes the motor control.

   @param OSSMRefBoardV2 Pointer to the OSSMRefBoardV2 properties.
  */
  /**************************************************************************/
  void begin(OSSMRefBoardV2Properties *OSSMRefBoardV2)
  {
    _motor = OSSMRefBoardV2;

    // Setup FastAccelStepper
    engine.init();
    _stepper = engine.stepperConnectToPin(_motor->stepPin);
    if (_stepper)
    {
      _stepper->setDirectionPin(_motor->directionPin, !_invertDirection);
      _stepper->setEnablePin(_motor->enablePin, _motor->enableActiveLow);
      _stepper->setAutoEnable(false);
      _stepper->disableOutputs();
      ESP_LOGI("OSSMRefBoardV2", "FastAccelStepper Engine successfully initialized!");
    }
    else
    {
      ESP_LOGE("OSSMRefBoardV2", "Failed to load FastAccelStepper Engine!");
    }

    pinMode(_motor->alarmPin, INPUT_PULLUP);
    pinMode(_motor->inPositionPin, INPUT_PULLUP);
    analogReadResolution(12);
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines steps per millimeter of travel. This is used
    to translate between metric units and steps.
    @param  stepsPerMillimeter steps per millimeter of travel. Default is 50.
  */
  /**************************************************************************/
  void setStepsPerMillimeter(int stepsPerMillimeter = 50)
  {
    _stepsPerMillimeter = stepsPerMillimeter;
    _maxStep = int(0.5 + _maxPosition * _stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _stepsPerMillimeter);
    ESP_LOGD("OSSMRefBoardV2", "Steps per millimeter set to %i", _stepsPerMillimeter);
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overall mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void setMachineGeometry(float travel, float keepout = 5.0)
  {
    _travel = travel;
    _keepout = keepout;
    _maxPosition = travel - (keepout * 2);
    _minStep = 0;
    _maxStep = int(0.5 + _maxPosition * _stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _stepsPerMillimeter);
    ESP_LOGD("OSSMRefBoardV2", "Machine Geometry Travel = %f", _travel);
    ESP_LOGD("OSSMRefBoardV2", "Machine Geometry Keepout = %f", _keepout);
    ESP_LOGD("OSSMRefBoardV2", "Machine Geometry MaxPosition = %f", _maxPosition);
  }

  /**************************************************************************/
  /*!
    @brief  Sets up sensorless homing.
    @param threshold Current threshold to account as mechanical endstop in [A].
    @param speed Speed of the homing procedure in [mm/s]. Default is 5.0.
  */
  /**************************************************************************/
  void setSensorlessHoming(float threshold = 0.10, float speed = 5.0)
  {
    _currentThreshold = threshold;
    _homingSpeed = speed * _stepsPerMillimeter;
    ESP_LOGI("OSSMRefBoardV2", "Search home with %05.1f mm/s.", speed);
  }

  /**************************************************************************/
  /*!
    @brief Homes the machine. This is done by moving the motor towards the
    homing switch until the current threshold is reached. The homing switch
    is then set as the new home position.
  */
  /**************************************************************************/
  void home()
  {
    // set homed to false so that isActive() becomes false
    _homed = false;

    // first stop current motion and suspend motion tasks
    stopMotion();

    // Quit if stepper not enabled
    if (_enabled == false)
    {
      ESP_LOGE("OSSMRefBoardV2", "Homing not possible! --> Enable stepper first!");
      return;
    }

    // Create homing task
    xTaskCreatePinnedToCore(
        this->_homingProcedureImpl, // Function that should be called
        "Homing",                   // Name of the task (for debugging)
        4096,                       // Stack size (bytes)
        this,                       // Pass reference to this class instance
        1,                          // Pretty high task priority
        &_taskHomingHandle,         // Task handle
        1                           // Have it on application core
    );
    ESP_LOGD("OSSMRefBoardV2", "Created Homing Task.");
  }

  /**************************************************************************/
  /*!
   @brief Measures the length of the rail.

   This measures the length of the rail by moving the motor back and forth until the endstop is reached on both sides.
   The length is then calculated from the number of steps and the steps per millimeter and stored in the motor properties using setMachineGeometry().
   @param callBackMeasuring Callback function that is called when measuring is completed. The callback function must take a float as an argument.
   @param keepout This keepout [mm] is a soft endstop and subtracted at both ends of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void measureRailLength(MeasureCallbackType callBackMeasuring, float keepout = 5.0)
  {
    // Quit if stepper not enabled
    if (_enabled == false)
    {
      ESP_LOGE("OSSMRefBoardV2", "Measuring not possible! --> Enable stepper first!");
      return;
    }

    // store the call back function
    _callBackMeasuring = callBackMeasuring;

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

  /**************************************************************************/
  /*!
    @brief Enables the stepper motor.
  */
  /**************************************************************************/
  void enable()
  {
    ESP_LOGI("OSSMRefBoardV2", "Stepper Enabled!");
    // Enable stepper
    _enabled = true;
    _stepper->enableOutputs();
  }

  /**************************************************************************/
  /*!
    @brief  Disable the stepper motor.
  */
  /**************************************************************************/
  void disable()
  {
    ESP_LOGI("OSSMRefBoardV2", "Stepper Disabled!");
    // Disable stepper
    _enabled = false;
    _stepper->disableOutputs();

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
      vTaskDelete(_taskHomingHandle);
      _taskHomingHandle = NULL;
      ESP_LOGD("OSSMRefBoardV2", "Deleted Homing Task.");
    }
  }

  /**************************************************************************/
  /*!
    @brief  Stops any motion as fast as legally possible.
  */
  /**************************************************************************/
  void stopMotion()
  {

    ESP_LOGW("OSSMRefBoardV2", "STOP MOTION!");

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
      vTaskDelete(_taskHomingHandle);
      _taskHomingHandle = NULL;
      ESP_LOGD("OSSMRefBoardV2", "Deleted Homing Task.");
    }

    // Delete measuring task should the measuring sequence be running
    if (_taskMeasuringHandle != NULL)
    {
      vTaskDelete(_taskMeasuringHandle);
      _taskMeasuringHandle = NULL;
      ESP_LOGD("OSSMRefBoardV2", "Deleted Measuring Task.");
    }

    if (_stepper->isRunning())
    {
      // Stop servo motor as fast as legally allowed
      _stepper->setAcceleration(_maxStepAcceleration);
      _stepper->applySpeedAcceleration();
      _stepper->stopMove();
      ESP_LOGD("OSSMRefBoardV2", "Bring stepper to a safe halt.");
    }

    // Wait for servo stopped
    while (_stepper->isRunning())
      ;
  }

  /**************************************************************************/
  /*!
    @brief  Returns if a trapezoidal motion is carried out, or the machine is
    at stand-still.
    @return `true` if motion is completed, `false` if still under way
  */
  /**************************************************************************/
  bool motionCompleted() { return _stepper->isRunning() ? false : true; }

  /**************************************************************************/
  /*!
    @brief  Returns the currently used acceleration.
    @return acceleration of the motor in [mm/s²]
  */
  /**************************************************************************/
  float getAcceleration() { return float(_stepper->getAcceleration()) / float(_stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current speed the machine.
    @return speed of the motor in [mm/s]
  */
  /**************************************************************************/
  float getSpeed() { return (float(_stepper->getCurrentSpeedInMilliHz()) * 1.0e-3) / float(_stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current position of the machine.
    @return position in [mm]
  */
  /**************************************************************************/
  float getPosition() { return float(_stepper->getCurrentPosition()) / float(_stepsPerMillimeter); }

  /**************************************************************************/
  /*!
   @brief Measures the current consumption.
   @param samples Number of samples to average.
   @return The current in A.
  */
  /**************************************************************************/
  float getCurrent(int samples = 20)
  {
    // iterate over the samples and sum up the ADC readings
    int sum = 0;
    for (int i = 0; i < samples; i++)
    {
      sum += analogReadMilliVolts(_motor->ADCPinCurrent) - _motor->AmpereOffsetInmV;
    }
    // calculate the average ADC reading
    float average = float(sum) / float(samples);
    // scale the value
    float current = average * _motor->AmperePermV;

    return current;
  }

  /**************************************************************************/
  /*!
   @brief Measures the voltage.
   @param samples Number of samples to average.
   @return The voltage in V.
  */
  /**************************************************************************/
  float getVoltage(int samples = 20)
  {
    // iterate over the samples and sum up the ADC readings
    int sum = 0;
    for (int i = 0; i < samples; i++)
    {
      sum += analogReadMilliVolts(_motor->ADCPinVoltage);
    }
    // calculate the average ADC reading
    float average = float(sum) / float(samples);
    // scale the value
    float voltage = average * _motor->VoltPermV;

    return voltage;
  }

  /**************************************************************************/
  /*!
   @brief Measures the power consumption.
   @param samples Number of samples to average.
   @return The power in W.
  */
  /**************************************************************************/
  float getPower(int samples = 20)
  {
    // iterate over the samples and sum up the ADC readings for current and voltage
    int sumCurrent = 0;
    int sumVoltage = 0;
    for (int i = 0; i < samples; i++)
    {
      sumCurrent += analogReadMilliVolts(_motor->ADCPinCurrent) - _motor->AmpereOffsetInmV;
      sumVoltage += analogReadMilliVolts(_motor->ADCPinVoltage);
    }
    // calculate the average ADC reading
    float averageCurrent = float(sumCurrent) / float(samples);
    float averageVoltage = float(sumVoltage) / float(samples);
    // subtract the offset and scale the value
    float current = averageCurrent * _motor->AmperePermV;
    float voltage = averageVoltage * _motor->VoltPermV;
    // calculate the power
    float power = current * voltage;

    return power;
  }

  /**************************************************************************/
  /*!
    @brief  Returns the data labels for valueA and valueB of a motion point.
    @return MotionPointLabel struct with the labels for valueA and valueB
  */
  /**************************************************************************/
  MotionPointLabel getMotionPointLabel()
  {
    MotionPointLabel label;
    label.labelValueA = "Current [A]";
    label.labelValueB = "Voltage [V]";
    return label;
  }

  /**************************************************************************/
  /*!
    @brief  Returns the error state of the motor. This is `true` if the motor
    is in an error state and `false` if everything is fine.
    @return error state
  */
  /**************************************************************************/
  int hasError()
  {
    // Check if alarm is active
    if (digitalRead(_motor->alarmPin) == LOW)
    {
      ESP_LOGE("OSSMRefBoardV2", "Alarm signal is active!");
      return 1;
    }
    return 0;
  }

private:
  /**************************************************************************/
  /*!
    @brief  Internal function that updates the trapezoidal motion path
    generator. Here this is done by calling the appropriate FastAccelStepper
    API calls and translate between metric units and steps
    @param position in [mm]
    @param speed in [mm/s]
    @param acceleration in [mm/s²]
  */
  /**************************************************************************/
  void _unsafeGoToPosition(float position, float speed, float acceleration)
  {
    // Translate between metric and steps
    int speedInHz = int(0.5 + speed * _stepsPerMillimeter);
    int stepAcceleration = int(0.5 + acceleration * _stepsPerMillimeter);
    int positionInSteps = int(0.5 + position * _stepsPerMillimeter);
    ESP_LOGD("OSSMRefBoardV2", "Going to unsafe position %i steps @ %i steps/s, %i steps/s^2", positionInSteps, speedInHz, stepAcceleration);

    // write values to stepper
    _stepper->setSpeedInHz(speedInHz);
    _stepper->setAcceleration(stepAcceleration);
    _stepper->moveTo(positionInSteps);
  }

  bool _queryHome()
  {
    // read current
    float current = getCurrent();
    // check if current is above threshold
    ESP_LOGV("OSSMRefBoardV2", "Readout current sensor: %.3f A, Threshold: %.3f A", current, (_idleCurrent + _currentThreshold));
    if (current > (_idleCurrent + _currentThreshold))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  void _homingProcedure()
  {
    ESP_LOGI("iHSVServoV6", "Start searching for home.");

    // Set feedrate for homing and stop motor
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration);

    // measure idle current
    _idleCurrent = getCurrent(100);
    ESP_LOGI("OSSMRefBoardV2", "Idle Current: %.3f A", _idleCurrent);

    // Move maximum travel distance + 2*keepout towards the homing switch
    _stepper->runBackward();

    // Poll homing switch
    while (_stepper->isRunning())
    {

      // Are we at the home position?
      if (_queryHome())
      {
        ESP_LOGD("OSSMRefBoardV2", "Found home!");
        // Set home position
        // Switch is at -KEEPOUT
        _stepper->forceStopAndNewPosition(_stepsPerMillimeter * int(-_keepout));

        // drive free of switch and set axis to lower end
        _stepper->moveTo(_minStep);

        // drive free of switch and set axis to 0
        _stepper->moveTo(0, true); // blocking call

        _homed = true;

        // Break loop, home was found
        break;
      }

      // Pause the task for 20ms to allow other tasks
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // disable Servo if homing has not found the homing switch
    if (!_homed)
    {
      _stepper->disableOutputs();
      ESP_LOGE("OSSMRefBoardV2", "Homing failed! Did not find home position.");
    }

    // Call notification callback, if it was defined.
    if (_callBackHoming != NULL)
    {
      _callBackHoming(_homed);
    }

    // delete one-time task
    _taskHomingHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("OSSMRefBoardV2", "Homing task self-terminated");
  }

  void _measureProcedure()
  {
    float travel = 0.0;

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
    _stepper->runForward();

    // wait until the motor is in position
    while (_stepper->isRunning())
    {
      // query endstop
      if (_queryHome())
      {
        // stop the motor
        _stepper->stopMove();

        // set the current position as the travel length. Add one keepout to account for homing on one side.
        float travel = getPosition() + _keepout;
        ESP_LOGI("OSSMRefBoardV2", "Measured rail length: %f", travel);
        setMachineGeometry(travel, _keepout);

        // drive free of end to _maxPosition
        _stepper->moveTo(_maxStep);

        break;
      }

      // Pause the task for 20ms to allow other tasks
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // Call notification callback, if it was defined.
    if (_callBackMeasuring != NULL)
    {
      _callBackMeasuring(travel);
    }

    // delete one-time task
    _taskMeasuringHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("OSSMRefBoardV2", "Measuring task self-terminated");
  }

  void _reportMotionPoint()
  {
    // Read alarm pin of the servo
    if (digitalRead(_motor->alarmPin) == LOW)
    {
      _error = true;
      ESP_LOGE("OSSMRefBoardV2", "Alarm signal is active!");
    }

    // Return results of current motion point via the callback
    if (_cbMotionPoint != NULL)
      _cbMotionPoint(
          millis(),
          getPosition(),
          getSpeed(),
          getCurrent(),
          getVoltage());
  }

  OSSMRefBoardV2Properties *_motor;
  FastAccelStepper *_stepper;
  FastAccelStepperEngine engine = FastAccelStepperEngine();
  float _idleCurrent;
  int _stepsPerMillimeter = 50;
  int _minStep;
  int _maxStep;
  int _maxStepPerSecond;
  int _maxStepAcceleration;
  static void _homingProcedureImpl(void *_this) { static_cast<OSSMRefBoardV2Motor *>(_this)->_homingProcedure(); }
  int _homingSpeed;
  TaskHandle_t _taskHomingHandle = NULL;
  static void _measureProcedureImpl(void *_this) { static_cast<OSSMRefBoardV2Motor *>(_this)->_measureProcedure(); }
  TaskHandle_t _taskMeasuringHandle = NULL;
  MeasureCallbackType _callBackMeasuring;
  float _currentThreshold; /*> Current threshold to count as hitting into an mechanical stop */
};
