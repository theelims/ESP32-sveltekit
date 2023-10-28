/**
 *   Abstract Motor Driver of StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "Arduino.h"
// #include <exception.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
// #include "freertos/semphr.h"

/**
 *  Motor
 *
 *  position = mm
 *  speed = mm/s
 *  acceleration = mm/s^2
 */

// class MotorException extends std::exception { }

/**************************************************************************/
/*!
  @brief  Motor Interface is a abstract class providing a possibility to
  abstract the motor from StrokeEngine. It provides a minimum set of functions
  that will be called by StrokeEngine. Costume implementations must take care
  of the trapezoidal motion path planning, as well as homing. The MotorInterface
  class is also accessible from the user code allowing further functions that
  a motor controller may offer.
*/
/**************************************************************************/
class MotorInterface
{
public:
  MotorInterface() {}

  /**************************************************************************/
  /*!
    @brief  Enables the motor driver. Meaning that the coils are energized and
    the motor is ready to accept motion commands.
  */
  /**************************************************************************/
  virtual void enable() = 0;

  /**************************************************************************/
  /*!
    @brief  Disables the motor driver. Motor must become powerless.
  */
  /**************************************************************************/
  virtual void disable() = 0;

  /**************************************************************************/
  /*!
    @brief  Returns the enable state of the motor.
    @return enable state
  */
  /**************************************************************************/
  virtual bool isEnabled() { return _enabled; }

  /**************************************************************************/
  /*!
    @brief  Returns the homed state of the motor.
    @return homed state
  */
  /**************************************************************************/
  virtual bool isHomed() { return _homed; }

  /**************************************************************************/
  /*!
    @brief  Returns the active state of the motor. This is true if the motor
    is enabled and is homed. This must be satisfied to issue move commands to
    the motor.
    @return active state
  */
  /**************************************************************************/
  virtual bool isActive() { return (_enabled && _homed); }

  /*     SemaphoreHandle_t claimMotorControl() {
        if (taskSemaphore == null) {
          taskSemaphore = xSemaphoreCreateBinary();
        }

        if( xSemaphoreTake( taskSemaphore, 100 / portTICK_PERIOD_MS ) != pdTRUE ) {
          throw new MotorBusyError("Unable to attach a new Motion Task, as one is already active!");
        }

        motionTask = xGetCurrentTaskHandle();
      }
      void releaseMotorControl(SemaphoreHandle_t semaphore) {
        if (semaphore != taskSemaphore) {
          throw new MotorGenericError("Attempted to release Semaphore using invalid handle!");
        }

        xSemaphoreGive(taskSemaphore);
        motionTask = null;
      } */

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  virtual void setMachineGeometry(float travel, float keepout = 5.0)
  {
    _travel = travel;
    _keepout = keepout;
    _maxPosition = travel - (keepout * 2.0);
    ESP_LOGD("AbstractMotor", "Machine Geometry Travel = %f", _travel);
    ESP_LOGD("AbstractMotor", "Machine Geometry Keepout = %f", _keepout);
    ESP_LOGD("AbstractMotor", "Machine Geometry MaxPosition = %f", _maxPosition);
  };

  /**************************************************************************/
  /*!
    @brief  Returns the maximum position the machine can safely travel. This
    is calculated from the values given by `setMachineGeometry()`. Minimum
    position is always at 0mm.
    @return maximum position in [mm]
  */
  /**************************************************************************/
  float getMaxPosition() { return _maxPosition; }

  /**************************************************************************/
  /*!
    @brief  Sets the maximum allowed speed the machine is capable.
    @param speed maximum allowed speed of the motor in [mm/s]
  */
  /**************************************************************************/
  virtual void setMaxSpeed(float speed)
  {
    _maxSpeed = speed;
    ESP_LOGD("AbstractMotor", "Set maximum speed to %05.2f mm/s", _maxSpeed);
  }

  /**************************************************************************/
  /*!
    @brief  Returns the maximum allowed speed the machine is capable.
    @return maximum allowed speed of the motor in [mm/s]
  */
  /**************************************************************************/
  float getMaxSpeed() { return _maxSpeed; }

  /**************************************************************************/
  /*!
    @brief  Sets the maximum allowed acceleration the machine is capable.
    @param acceleration maximum allowed acceleration of the motor in [mm/s²]
  */
  /**************************************************************************/
  virtual void setMaxAcceleration(float acceleration)
  {
    _maxAcceleration = acceleration;
    ESP_LOGD("AbstractMotor", "Set maximum acceleration to %05.2f mm/s^2", _maxAcceleration);
  }

  /**************************************************************************/
  /*!
    @brief  Returns the maximum allowed acceleration the machine is capable.
    @return maximum allowed acceleration of the motor in [mm/s²]
  */
  /**************************************************************************/
  float getMaxAcceleration() { return _maxAcceleration; }

  /**************************************************************************/
  /*!
    @brief  This invokes a trapezoidal motion path planning and
    execution. It includes basic safeguards against nonphysical inputs and
    clips postion, speed and acceleration to the maximum values specified for
    the particular motor. The function call is ignored if the motor is neither
    enabled and homed. The trapezoidal motion path planning may be called even
    amidst a running stroke and updates the motion profile. If necessary even
    decelerating to stand-still and reversing the direction.
    @param position Target position that must be reached starting at the current
    position and speed in [mm]
    @param speed The speed at which the motion should happen [mm/s]
    @param acceleration The acceleration that is used to get up to speed and
    to slow down at the end in [mm/s²]
  */
  /**************************************************************************/
  void goToPosition(float position, float speed, float acceleration)
  {
    // TODO - If a motion task is provided, ensure the caller is the motion task (Mutex?)
    // Ensure in ACTIVE and valid movement state
    if (!_enabled || !_homed)
    {
      // throw new MotorInvalidStateError("Unable to command motion while motor is not ENABLED or HOMED!");
      ESP_LOGE("AbstractMotor", "Unable to command motion while motor is not ENABLED or HOMED!");
    }

    /*       // Take Semaphore for movement
          if( xSemaphoreTake( movementSemaphore, 1000 / portTICK_PERIOD_MS ) != pdTRUE ) {
            throw new MotorInMotionError("Unable to acquire Motor Movement Semaphore within 1000ms. Motor currently within movement still!");
          } */

    // Apply bounds and protections
    float safePosition = constrain(position, 0.0, _maxPosition);
    float safeSpeed = constrain(speed, 0.0, _maxSpeed);
    float safeAcceleration = constrain(acceleration, 0.0, _maxAcceleration);

    if (safePosition != position)
    {
      ESP_LOGW("AbstractMotor", "Clipped position to fit within bounds! %05.1f was clipped to %05.1f", position, safePosition);
    }

    if (safeSpeed != speed)
    {
      ESP_LOGW("AbstractMotor", "Clipped speed to fit within bounds! %05.1f was clipped to %05.1f", speed, safeSpeed);
    }

    if (safeAcceleration != acceleration)
    {
      ESP_LOGW("AbstractMotor", "Clipped acceleration to fit within bounds! %05.1f was clipped to %05.1f", acceleration, safeAcceleration);
    }

    ESP_LOGD("AbstractMotor", "Going to position %05.1f mm @ %05.1f mm/s, %05.1f mm/s^2", safePosition, safeSpeed, safeAcceleration);
    _unsafeGoToPosition(safePosition, safeSpeed, safeAcceleration);
  }

  /**************************************************************************/
  /*!
    @brief  Initiates the fastest safe breaking to stand-still stopping all
    motion without loosing position.
  */
  /**************************************************************************/
  virtual void stopMotion() = 0;

  /**************************************************************************/
  /*!
    @brief  Returns if a trapezoidal motion is carried out, or the machine is
    at stand-still.
    @return `true` if motion is completed, `false` if still under way
  */
  /**************************************************************************/
  virtual bool motionCompleted() = 0;

  /**************************************************************************/
  /*!
    @brief  Returns the currently used acceleration.
    @return acceleration of the motor in [mm/s²]
  */
  /**************************************************************************/
  virtual float getAcceleration() = 0;

  /**************************************************************************/
  /*!
    @brief  Returns the current speed the machine.
    @return speed of the motor in [mm/s]
  */
  /**************************************************************************/
  virtual float getSpeed() = 0;

  /**************************************************************************/
  /*!
    @brief  Returns the current position of the machine.
    @return position in [mm]
  */
  /**************************************************************************/
  virtual float getPosition() = 0;

protected:
  bool _enabled = false;
  bool _homed = false;

  // TaskHandle_t motionTask; // Task which will provide movement commands to Motor
  SemaphoreHandle_t taskSemaphore;     // Prevent more than one task being interfaced to a motor at a time
  SemaphoreHandle_t movementSemaphore; // Prevent additional movement commands while in motion

  /**************************************************************************/
  /*!
    @brief  This function is getting called from within `goToPosition()` and
    must be overriden by the user implementation for a specific motor.
    @param position Target position that must be reached starting at the current
    position and speed in [mm]
    @param speed The speed at which the motion should happen [mm/s]
    @param acceleration The acceleration that is used to get up to speed and
    to slow down at the end in [mm/s²]
  */
  /**************************************************************************/
  virtual void _unsafeGoToPosition(float position, float speed, float acceleration) = 0;

  float _travel = 0.0;
  float _keepout = 0.0;
  float _maxPosition = 0.0;
  float _maxSpeed = 0.0;
  float _maxAcceleration = 0.0;
};
