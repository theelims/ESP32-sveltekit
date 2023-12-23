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
  virtual void begin(motorProperties *motor);

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void setMachineGeometry(float travel, float keepout = 5.0);

  void setSensoredHoming(int homePin, uint8_t pinMode = INPUT_PULLDOWN, bool activeLow = true); // Assumes always homing to back of machine for safety
  void home(float homePosition = 0.0, float speed = 5.0);
  void home(void (*callBackHoming)(bool), float homePosition = 0.0, float speed = 5.0);

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
  void attachPositionFeedback(void (*cbMotionPoint)(unsigned int, float, float), unsigned int timeInMs = 50);

  // Control
  void enable();
  void disable();

  // Motion
  void stopMotion();
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
  void _unsafeGoToPosition(float position, float speed, float acceleration);

  /**************************************************************************/
  /*!
    @brief  Returns the current position of the machine. Must be overriden if
    other forms of homing as end stop switches are desired.
    @return position in [mm]
  */
  /**************************************************************************/
  virtual bool _queryHome();

  /**************************************************************************/
  /*!
    @brief  Virtual function to setup the homing procedure in derived classes
  */
  /**************************************************************************/
  virtual void _beginHoming() {}

  /**************************************************************************/
  /*!
    @brief  A virtual function the is called once the motor has found the home
    position. This can be used by derived motor drivers to perform something
    useful.
  */
  /**************************************************************************/
  virtual void _atHome() {}

  /**************************************************************************/
  /*!
    @brief  A virtual function that calls the motion point callback function
  */
  /**************************************************************************/
  virtual void _callBackMotionPoint();

  /**************************************************************************/
  FastAccelStepper *_stepper;
  motorProperties *_motor;
  int _minStep;
  int _maxStep;
  int _maxStepPerSecond;
  int _maxStepAcceleration;
  bool _motionCompleted = true;
  static void _homingProcedureImpl(void *_this) { static_cast<GenericStepperMotor *>(_this)->_homingProcedure(); }
  void _homingProcedure();
  int _homingSpeed;
  float _homePosition;
  int _homingPin = -1;
  bool _homingActiveLow; /*> Polarity of the homing signal*/
  TaskHandle_t _taskHomingHandle = NULL;
  void (*_callBackHoming)(bool) = NULL;
  void (*_cbMotionPoint)(unsigned int, float, float) = NULL;
  TickType_t _timeSliceInMs = 50;
  static void _positionFeedbackTaskImpl(void *_this) { static_cast<GenericStepperMotor *>(_this)->_positionFeedbackTask(); }
  void _positionFeedbackTask();
  TaskHandle_t _taskPositionFeedbackHandle = NULL;
};
