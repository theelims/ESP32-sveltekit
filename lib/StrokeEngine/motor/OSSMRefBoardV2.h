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
  int stepsPerMillimeter; /*> Number of steps per millimeter */
  bool invertDirection;   /*> Set to true to invert the direction signal
                           *  The firmware expects the home switch to be located at the
                           *  end of an retraction move. That way the machine homes
                           *  itself away from the body. Home position is -KEEPOUTBOUNDARY */
  bool enableActiveLow;   /*> Polarity of the enable signal. True for active low. */
  int stepPin;            /*> Pin connected to the STEP input */
  int directionPin;       /*> Pin connected to the DIR input */
  int enablePin;          /*> Pin connected to the ENA input */
  int alarmPin;           /*> Pin connected to the ALM input */
  int inPositionPin;      /*> Pin connected to the PED input */
  int ADCPinCurrent;      /*> Pin connected to the current sensor input */
  float AmperePermV;      /*> Scaling of the current sensor ADC */
  int AmpereOffsetInmV;   /*> Offset in mV of the current sensor ADC */
  float currentThreshold; /*> Current threshold to count as hitting into an mechanical stop */
  int ADCPinVoltage;      /*> Pin connected to the voltage sensor input */
  float VoltPermV;        /*> Scaling of the voltage ADC */
} OSSMRefBoardV2Properties;

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
   * @brief Initializes the motor control.
   *
   * @param motor Pointer to the motor properties.
   * @param OSSMRefBoardV2 Pointer to the OSSMRefBoardV2 properties.
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
      _stepper->setDirectionPin(_motor->directionPin, _motor->invertDirection);
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
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
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
    _maxStep = int(0.5 + _maxPosition * _motor->stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _motor->stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _motor->stepsPerMillimeter);
    ESP_LOGD("OSSMRefBoardV2", "Machine Geometry Travel = %f", _travel);
    ESP_LOGD("OSSMRefBoardV2", "Machine Geometry Keepout = %f", _keepout);
    ESP_LOGD("OSSMRefBoardV2", "Machine Geometry MaxPosition = %f", _maxPosition);
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
  void home(float homePosition = 0.0, float speed = 5.0)
  {
    _homePosition = int(0.5 + homePosition / float(_motor->stepsPerMillimeter));
    _homingSpeed = speed * _motor->stepsPerMillimeter;
    ESP_LOGI("OSSMRefBoardV2", "Search home with %05.1f mm/s at %05.1f mm.", speed, homePosition);

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
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void home(void (*callBackHoming)(bool), float homePosition = 0.0, float speed = 5.0)
  {
    _callBackHoming = callBackHoming;
    home(homePosition, speed);
  }

  /**************************************************************************/
  /*!
   * @brief Measures the length of the rail.
   *
   * This measures the length of the rail by moving the motor back and forth until the in-position signal is reached on both sides.
   * The length is then calculated from the number of steps and the steps per millimeter and stored in the motor properties using setMachineGeometry().
   * @param keepout This keepout [mm] is a soft endstop and subtracted at both ends of the travel. A typical value would be 5mm.
   */
  /**************************************************************************/
  void measureRailLength(float keepout = 5.0)
  {
    // Quit if stepper not enabled
    if (_enabled == false)
    {
      ESP_LOGE("OSSMRefBoardV2", "Measuring not possible! --> Enable stepper first!");
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

  /**************************************************************************/
  /*!
    @brief  It also attaches a callback function where the speed and position
    are reported on a regular interval specified with timeInMs.
    @param cbMotionPoint Callback with the signature
    `cbMotionPoint(unsigned int timestamp, float position, float speed)`. time is reported
    milliseconds since the controller has started (`millis()`), speed in [m/s] and
    position in [mm].
    @param timeInMs time interval at which speed and position should be
    reported in [ms]
  */
  /**************************************************************************/
  void attachPositionFeedback(void (*cbMotionPoint)(unsigned int, float, float, float, float), unsigned int timeInMs = 50)
  {
    _cbMotionPoint = cbMotionPoint;
    _timeSliceInMs = timeInMs / portTICK_PERIOD_MS;
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void enable()
  {
    ESP_LOGI("OSSMRefBoardV2", "Stepper Enabled!");
    // Enable stepper
    _enabled = true;
    _stepper->enableOutputs();

    if (_cbMotionPoint == NULL)
    {
      ESP_LOGD("OSSMRefBoardV2", "No Position Feedback Task created.");
      return;
    }

    // Create / resume motion feedback task
    if (_taskPositionFeedbackHandle == NULL)
    {
      // Create Stroke Task
      xTaskCreatePinnedToCore(
          _positionFeedbackTaskImpl,    // Function that should be called
          "Motion Feedback",            // Name of the task (for debugging)
          4096,                         // Stack size (bytes)
          this,                         // Pass reference to this class instance
          10,                           // Pretty high task priority
          &_taskPositionFeedbackHandle, // Task handle
          1                             // Pin to application core
      );
      ESP_LOGD("OSSMRefBoardV2", "Created Position Feedback Task.");
    }
    else
    {
      // Resume task, if it already exists
      vTaskResume(_taskPositionFeedbackHandle);
      ESP_LOGD("OSSMRefBoardV2", "Resumed Position Feedback Task.");
    }
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
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

    // Suspend motion feedback task if it exists already
    if (_taskPositionFeedbackHandle != NULL)
    {
      vTaskSuspend(_taskPositionFeedbackHandle);
      ESP_LOGD("OSSMRefBoardV2", "Suspended Position Feedback Task.");
    }
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
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
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  bool motionCompleted() { return _stepper->isRunning() ? false : true; }

  /**************************************************************************/
  /*!
    @brief  Returns the currently used acceleration.
    @return acceleration of the motor in [mm/s²]
  */
  /**************************************************************************/
  float getAcceleration() { return float(_stepper->getAcceleration()) / float(_motor->stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current speed the machine.
    @return speed of the motor in [mm/s]
  */
  /**************************************************************************/
  float getSpeed() { return (float(_stepper->getCurrentSpeedInMilliHz()) * 1.0e-3) / float(_motor->stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current position of the machine.
    @return position in [mm]
  */
  /**************************************************************************/
  float getPosition() { return float(_stepper->getCurrentPosition()) / float(_motor->stepsPerMillimeter); }

  /**************************************************************************/
  /*!
   * @brief Gets the current value.
   *
   * @param samples Number of samples to average.
   * @return The current in A.
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
   * @brief Gets the voltage value.
   *
   * @param samples Number of samples to average.
   * @return The voltage in V.
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
   * @brief Gets the power value.
   *
   * @param samples Number of samples to average.
   * @return The power in W.
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
    int speedInHz = int(0.5 + speed * _motor->stepsPerMillimeter);
    int stepAcceleration = int(0.5 + acceleration * _motor->stepsPerMillimeter);
    int positionInSteps = int(0.5 + position * _motor->stepsPerMillimeter);
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
    ESP_LOGI("OSSMRefBoardV2", "Readout current sensor: %.3f A, Threshold: %.3f A", current, (_idleCurrent + _motor->currentThreshold));
    if (current > (_idleCurrent + _motor->currentThreshold))
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
    ESP_LOGD("OSSMRefBoardV2", "Start searching for home.");

    if (_stepper->isRunning())
    {
      ESP_LOGD("OSSMRefBoardV2", "Stepper is moving. Stop and home");
      stopMotion();
    }

    // Set feedrate for homing
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration / 10);

    // measure idle current
    _idleCurrent = getCurrent();
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
        _stepper->forceStopAndNewPosition(_motor->stepsPerMillimeter * int(_homePosition - _keepout));

        // drive free of switch and set axis to lower end
        _stepper->moveTo(_minStep);

        _homed = true;

        // drive free of switch and set axis to 0
        _stepper->moveTo(0);

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

  void _positionFeedbackTask()
  {
    TickType_t xLastWakeTime;
    // Initialize the xLastWakeTime variable with the current tick count.
    xLastWakeTime = xTaskGetTickCount();

    unsigned int now = millis();

    while (true)
    {
      // Return results of current motion point via the callback
      _cbMotionPoint(
          millis(),
          getPosition(),
          getSpeed(),
          getVoltage(),
          getCurrent());

      // Delay the task until the next tick count
      vTaskDelayUntil(&xLastWakeTime, _timeSliceInMs);
    }
  }

  OSSMRefBoardV2Properties *_motor;
  FastAccelStepper *_stepper;
  FastAccelStepperEngine engine = FastAccelStepperEngine();
  float _idleCurrent;
  int _minStep;
  int _maxStep;
  int _maxStepPerSecond;
  int _maxStepAcceleration;
  bool _motionCompleted = true;
  static void _homingProcedureImpl(void *_this) { static_cast<OSSMRefBoardV2Motor *>(_this)->_homingProcedure(); }
  int _homingSpeed;
  float _homePosition;
  TaskHandle_t _taskHomingHandle = NULL;
  void (*_callBackHoming)(bool) = NULL;
  TickType_t _timeSliceInMs = 50;
  static void _positionFeedbackTaskImpl(void *_this) { static_cast<OSSMRefBoardV2Motor *>(_this)->_positionFeedbackTask(); }
  TaskHandle_t _taskPositionFeedbackHandle = NULL;
  static void _measureProcedureImpl(void *_this) { static_cast<OSSMRefBoardV2Motor *>(_this)->_measureProcedure(); }
  TaskHandle_t _taskMeasuringHandle = NULL;
  void (*_cbMotionPoint)(unsigned int, float, float, float, float) = NULL;
};
