/**
 *   Generic Stepper Motor Driver of StrokeEngine
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
  linear motion.
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
} motorProperties;

/**************************************************************************/
/*!
  @brief  Generic Stepper inherits from MotorInterface and provides a generic
  STEP/DIR interface to all common stepper and servo drivers. Under the hood
  it uses FastAccelStepper for the trapezoidal motion planning and the
  hardware step signal generation. The generic stepper class uses a physical
  end switch for homing. However, the homing procedure is written in a way
  that derived classes may simply provide other signal inputs for homing.
*/
/**************************************************************************/
class GenericStepperMotor : public MotorInterface
{
public:
  GenericStepperMotor() {}

  // Init
  virtual void begin(motorProperties *motor)
  {
    _motor = motor;

    // Setup FastAccelStepper
    engine.init();
    _stepper = engine.stepperConnectToPin(_motor->stepPin);
    if (_stepper)
    {
      _stepper->setDirectionPin(_motor->directionPin, _motor->invertDirection);
      _stepper->setEnablePin(_motor->enablePin, _motor->enableActiveLow);
      _stepper->setAutoEnable(false);
      _stepper->disableOutputs();
      ESP_LOGI("GenericStepper", "FastAccelStepper Engine successfully initialized!");
    }
    else
    {
      ESP_LOGE("GenericStepper", "Failed to load FastAccelStepper Engine!");
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
  void setMachineGeometry(float travel, float keepout = 5.0)
  {
    _travel = travel;
    _keepout = keepout;
    _maxPosition = travel - (keepout * 2);
    _minStep = 0;
    _maxStep = int(0.5 + _maxPosition * _motor->stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _motor->stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _motor->stepsPerMillimeter);
    ESP_LOGD("GenericStepper", "Machine Geometry Travel = %f", _travel);
    ESP_LOGD("GenericStepper", "Machine Geometry Keepout = %f", _keepout);
    ESP_LOGD("GenericStepper", "Machine Geometry MaxPosition = %f", _maxPosition);
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
  void setSensoredHoming(int homePin, uint8_t arduinoPinMode = INPUT_PULLDOWN, bool activeLow = true) // Assumes always homing to back of machine for safety
  {
    // set homing pin as input
    _homingPin = homePin;
    pinMode(_homingPin, arduinoPinMode);
    _homingActiveLow = activeLow;
    ESP_LOGI("GenericStepper", "Homing switch on pin %i in pin mode %i is %s", _homingPin, arduinoPinMode, _homingActiveLow ? "active low" : "active high");
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
  void home(float homePosition = 0.0, float speed = 5.0)
  {
    _homePosition = int(0.5 + homePosition / float(_motor->stepsPerMillimeter));
    _homingSpeed = speed * _motor->stepsPerMillimeter;
    ESP_LOGI("GenericStepper", "Search home with %05.1f mm/s at %05.1f mm.", speed, homePosition);

    // set homed to false so that isActive() becomes false
    _homed = false;

    // first stop current motion and suspend motion tasks
    stopMotion();

    // Quit if stepper not enabled
    if (_enabled == false)
    {
      ESP_LOGE("GenericStepper", "Homing not possible! --> Enable stepper first!");
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
    ESP_LOGD("GenericStepper", "Created Homing Task.");
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
  void attachPositionFeedback(void (*cbMotionPoint)(unsigned int, float, float), unsigned int timeInMs = 50)
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
    ESP_LOGI("GenericStepper", "Stepper Enabled!");
    // Enable stepper
    _enabled = true;
    _stepper->enableOutputs();

    if (_cbMotionPoint == NULL)
    {
      ESP_LOGD("GenericStepper", "No Position Feedback Task created.");
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
      ESP_LOGD("GenericStepper", "Created Position Feedback Task.");
    }
    else
    {
      // Resume task, if it already exists
      vTaskResume(_taskPositionFeedbackHandle);
      ESP_LOGD("GenericStepper", "Resumed Position Feedback Task.");
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
    ESP_LOGI("GenericStepper", "Stepper Disabled!");
    // Disable stepper
    _enabled = false;
    _stepper->disableOutputs();

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
      vTaskDelete(_taskHomingHandle);
      _taskHomingHandle = NULL;
      ESP_LOGD("GenericStepper", "Deleted Homing Task.");
    }

    // Suspend motion feedback task if it exists already
    if (_taskPositionFeedbackHandle != NULL)
    {
      vTaskSuspend(_taskPositionFeedbackHandle);
      ESP_LOGD("GenericStepper", "Suspended Position Feedback Task.");
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

    ESP_LOGW("GenericStepper", "STOP MOTION!");

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
      vTaskDelete(_taskHomingHandle);
      _taskHomingHandle = NULL;
      ESP_LOGD("GenericStepper", "Deleted Homing Task.");
    }

    if (_stepper->isRunning())
    {
      // Stop servo motor as fast as legally allowed
      _stepper->setAcceleration(_maxStepAcceleration);
      _stepper->applySpeedAcceleration();
      _stepper->stopMove();
      ESP_LOGD("GenericStepper", "Bring stepper to a safe halt.");
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

  // Misc
  // FastAccelStepperEngine &fastAccelStepperEngineReference();

protected:
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
    ESP_LOGD("GenericStepper", "Going to unsafe position %i steps @ %i steps/s, %i steps/s^2", positionInSteps, speedInHz, stepAcceleration);

    // write values to stepper
    _stepper->setSpeedInHz(speedInHz);
    _stepper->setAcceleration(stepAcceleration);
    _stepper->moveTo(positionInSteps);
  }

  virtual bool _queryHome()
  {
    ESP_LOGI("GenericStepper", "Querying homing switch.");
    return (digitalRead(_homingPin) == !_homingActiveLow) ? true : false;
  }

  void _homingProcedure()
  {
    if (_stepper->isRunning())
    {
      ESP_LOGD("GenericStepper", "Stepper is moving. Stop and home");
      stopMotion();
    }

    // Set feedrate for homing
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration / 10);

    // Check if we are already at the home position
    if (_queryHome())
    {
      ESP_LOGD("GenericStepper", "Already at home position. Backing up and try again.");
      // back off 2*keepout from switch
      _stepper->move(_motor->stepsPerMillimeter * 2 * _keepout);

      // wait for move to complete
      while (_stepper->isRunning())
      {
        // Pause the task for 100ms while waiting for move to complete
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

      // move back towards endstop
      _stepper->move(-_motor->stepsPerMillimeter * 4 * _keepout);
    }
    else
    {
      ESP_LOGD("GenericStepper", "Start searching for home.");
      // Move maximum travel distance + 2*keepout towards the homing switch
      _stepper->move(-_motor->stepsPerMillimeter * (_maxPosition + 4 * _keepout));
    }

    // Poll homing switch
    while (_stepper->isRunning())
    {

      // Are we at the home position?
      if (_queryHome())
      {
        ESP_LOGD("GenericStepper", "Found home!");
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
      ESP_LOGE("GenericStepper", "Homing failed! Did not find home position.");
    }

    // Call notification callback, if it was defined.
    if (_callBackHoming != NULL)
    {
      _callBackHoming(_homed);
    }

    // delete one-time task
    _taskHomingHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("GenericStepper", "Homing task self-terminated");
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
          getSpeed());

      // Delay the task until the next tick count
      vTaskDelayUntil(&xLastWakeTime, _timeSliceInMs);
    }
  }

  /**************************************************************************/
  FastAccelStepper *_stepper;
  motorProperties *_motor;
  FastAccelStepperEngine engine = FastAccelStepperEngine();
  int _minStep;
  int _maxStep;
  int _maxStepPerSecond;
  int _maxStepAcceleration;
  bool _motionCompleted = true;
  static void _homingProcedureImpl(void *_this) { static_cast<GenericStepperMotor *>(_this)->_homingProcedure(); }
  int _homingSpeed;
  float _homePosition;
  int _homingPin = -1;
  bool _homingActiveLow; /*> Polarity of the homing signal*/
  TaskHandle_t _taskHomingHandle = NULL;
  void (*_callBackHoming)(bool) = NULL;
  void (*_cbMotionPoint)(unsigned int, float, float) = NULL;
  TickType_t _timeSliceInMs = 50;
  static void _positionFeedbackTaskImpl(void *_this) { static_cast<GenericStepperMotor *>(_this)->_positionFeedbackTask(); }
  TaskHandle_t _taskPositionFeedbackHandle = NULL;
};
