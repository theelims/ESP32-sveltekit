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
  bool enableActiveLow; /*> Polarity of the enable signal. True for active low. */
  int stepPin;          /*> Pin connected to the STEP input */
  int directionPin;     /*> Pin connected to the DIR input */
  int enablePin;        /*> Pin connected to the ENA input */
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
  void begin(motorProperties *motor)
  {
    _motor = motor;

    // Setup FastAccelStepper
    engine.init();
    _stepper = engine.stepperConnectToPin(_motor->stepPin);
    if (_stepper)
    {
      _stepper->setDirectionPin(_motor->directionPin, !_invertDirection);
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
    _maxStep = int(0.5 + _maxPosition * _stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _stepsPerMillimeter);
    ESP_LOGD("GenericStepper", "Machine Geometry Travel = %f", _travel);
    ESP_LOGD("GenericStepper", "Machine Geometry Keepout = %f", _keepout);
    ESP_LOGD("GenericStepper", "Machine Geometry MaxPosition = %f", _maxPosition);
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines steps per millimeter of travel. This is used
    to translate between metric units and steps.
    @param  stepsPerMillimeter steps per millimeter of travel.
  */
  /**************************************************************************/
  void setStepsPerMillimeter(int stepsPerMillimeter = 50)
  {
    _stepsPerMillimeter = stepsPerMillimeter;
    _maxStep = int(0.5 + _maxPosition * _stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _stepsPerMillimeter);
    ESP_LOGD("GenericStepper", "Steps per Millimeter = %i", _stepsPerMillimeter);
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
    @param homePosition Position of the homing switch in [mm]. Default is 0.0.
    @param speed Speed of the homing procedure in [mm/s]. Default is 5.0.
  */
  /**************************************************************************/
  void setSensoredHoming(int homePin, uint8_t arduinoPinMode = INPUT_PULLDOWN, bool activeLow = true, float homePosition = 0.0, float speed = 5.0) // Assumes always homing to back of machine for safety
  {
    // set homing pin as input
    _homingPin = homePin;
    pinMode(_homingPin, arduinoPinMode);
    _homingActiveLow = activeLow;
    _homePosition = int(0.5 + homePosition / float(_stepsPerMillimeter));
    _homingSpeed = speed * _stepsPerMillimeter;
    ESP_LOGI("GenericStepper", "Homing switch on pin %i in pin mode %i is %s", _homingPin, arduinoPinMode, _homingActiveLow ? "active low" : "active high");
    ESP_LOGI("GenericStepper", "Search home with %05.1f mm/s at %05.1f mm.", speed, homePosition);
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
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
  void enable()
  {
    ESP_LOGI("GenericStepper", "Stepper Enabled!");
    // Enable stepper
    _enabled = true;
    _stepper->enableOutputs();
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
    @brief  Initiates the fastest safe breaking to stand-still stopping all
    motion without loosing position.
  */
  /**************************************************************************/
  void stopMotion()
  {

    ESP_LOGW("GenericStepper", "STOP MOTION!");

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
      ESP_LOGD("GenericStepper", "Deleted Homing Task: %p", _taskHomingHandle);
      vTaskDelete(_taskHomingHandle);
      _taskHomingHandle = NULL;
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

  // Misc
  // FastAccelStepperEngine &fastAccelStepperEngineReference();

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
    ESP_LOGD("GenericStepper", "Going to unsafe position %i steps @ %i steps/s, %i steps/s^2", positionInSteps, speedInHz, stepAcceleration);

    // write values to stepper
    _stepper->setSpeedInHz(speedInHz);
    _stepper->setAcceleration(stepAcceleration);
    _stepper->moveTo(positionInSteps);
  }

  void _reportMotionPoint()
  {
    // Call notification callback, if it was defined.
    if (_cbMotionPoint != NULL)
    {
      _cbMotionPoint(millis(), getPosition(), getSpeed(), 0.0, 0.0);
    }
  }

  bool _queryHome()
  {
    ESP_LOGV("GenericStepper", "Querying homing switch.");
    return (digitalRead(_homingPin) == !_homingActiveLow) ? true : false;
  }

  void _homingProcedure()
  {
    // Set feedrate for homing
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration);

    // Check if we are already at the home position
    if (_queryHome())
    {
      ESP_LOGD("GenericStepper", "Already at home position. Backing up and try again.");
      // back off 2*keepout from switch
      _stepper->move(_stepsPerMillimeter * 2 * _keepout);

      // wait for move to complete
      while (_stepper->isRunning())
      {
        // Pause the task for 100ms while waiting for move to complete
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

      // move back towards endstop
      _stepper->move(-_stepsPerMillimeter * 4 * _keepout);
    }
    else
    {
      ESP_LOGD("GenericStepper", "Start searching for home.");
      // Move maximum travel distance + 2*keepout towards the homing switch
      _stepper->move(-_stepsPerMillimeter * (_maxPosition + 4 * _keepout));
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
        _stepper->forceStopAndNewPosition(_stepsPerMillimeter * int(_homePosition - _keepout));

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
      _callBackHoming();
    }

    // delete one-time task
    _taskHomingHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("GenericStepper", "Homing task self-terminated");
  }

  /**************************************************************************/
  FastAccelStepper *_stepper;
  motorProperties *_motor;
  FastAccelStepperEngine engine = FastAccelStepperEngine();
  int _stepsPerMillimeter = 50;
  int _minStep;
  int _maxStep;
  int _maxStepPerSecond;
  int _maxStepAcceleration;
  static void _homingProcedureImpl(void *_this) { static_cast<GenericStepperMotor *>(_this)->_homingProcedure(); }
  int _homingSpeed;
  float _homePosition;
  int _homingPin = -1;
  bool _homingActiveLow; /*> Polarity of the homing signal*/
  TaskHandle_t _taskHomingHandle = NULL;
};
