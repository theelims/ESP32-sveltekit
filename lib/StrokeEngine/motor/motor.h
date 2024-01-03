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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/**************************************************************************/
/*!
  @brief  This struct is used to pass data labels for valueA and valueB of a
  motion point.
*/
/**************************************************************************/
typedef struct
{
  String labelValueA;
  String labelValueB;
} MotionPointLabel;

using HomingCallbackType = std::function<void(boolean)>;

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
    @brief  Initializes the motor driver. This must be called as first
  */
  /**************************************************************************/
  virtual void begin() {}

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
    @brief  Homes the motor. This is a non-blocking function.
  */
  /**************************************************************************/
  virtual void home() = 0;

  /**************************************************************************/
  /*!
    @brief Homes the machine. This is a non-blocking function and notifies
    the caller via the callback function when homing is completed.
    @param callBackHoming Callback function that is called when homing is
    completed. The callback function must take a boolean as an argument.
  */
  /**************************************************************************/
  void home(HomingCallbackType callBackHoming)
  {
    _callBackHoming = callBackHoming;
    home();
  }

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
    @brief  Returns the mechanical (measured) travel of the machine from endstop to endstop.
    @return travel in [mm]
  */
  /**************************************************************************/
  float getTravel() { return _travel; }

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
    @brief Inverts the direction of the motor. This is useful if the motor
    is mounted in a way that the direction of the motor is inverted.
    @param invert `true` if the direction should be inverted, `false` if not
  */
  /**************************************************************************/
  void invertDirection(boolean invert = true)
  {
    _invertDirection = invert;
    ESP_LOGD("AbstractMotor", "Set invert direction to %d", _invertDirection);
  }

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
    if (!_enabled || !_homed || _error)
    {
      ESP_LOGE("AbstractMotor", "Unable to command motion while motor is not ENABLED, nor HOMED or in ERROR state!");
      return;
    }

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

  /**************************************************************************/
  /*!
    @brief  It also attaches a callback function where the speed and position
    are reported on a regular interval specified with timeInMs.
    @param cbMotionPoint Callback with the signature
    `cbMotionPoint(unsigned int timestamp, float position, float speed, float valueA,
    float valueB)`. time is reported milliseconds since the controller has started
    (`millis()`), speed in [m/s] and position in [mm]. valueA & valueB can be
    arbitrary data like current, voltage, real position, torque, etc.
    @param timeInMs time interval at which speed and position should be
    reported in [ms]
  */
  /**************************************************************************/
  void attachPositionFeedback(void (*cbMotionPoint)(unsigned int, float, float, float, float), unsigned int timeInMs = 50)
  {
    _cbMotionPoint = cbMotionPoint;
    _timeSliceInMs = timeInMs / portTICK_PERIOD_MS;

    // Create / resume motion feedback task
    if (_taskPositionFeedbackHandle == NULL)
    {
      // Create Feedback Task
      xTaskCreatePinnedToCore(
          _positionFeedbackTaskImpl,    // Function that should be called
          "Motion Feedback",            // Name of the task (for debugging)
          4096,                         // Stack size (bytes)
          this,                         // Pass reference to this class instance
          20,                           // Pretty high task priority
          &_taskPositionFeedbackHandle, // Task handle
          1                             // Pin to application core
      );
      ESP_LOGD("AbstractMotor", "Created Position Feedback Task.");
    }
  }

  /**************************************************************************/
  /*!
   @brief Detaches the position feedback for the motor. This function sets the
   motion point callback to NULL and deletes the task responsible for position
   feedback.
  */
  /**************************************************************************/
  void detachPositionFeedback()
  {
    _cbMotionPoint = NULL;
    vTaskDelete(_taskPositionFeedbackHandle);
    _taskPositionFeedbackHandle = NULL;
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
    label.labelValueA = "None";
    label.labelValueB = "None";
    return label;
  }

  /**************************************************************************/
  /*!
    @brief  Returns the error state of the motor. This is `true` if the motor
    is in an error state and `false` if everything is fine. Can be overriden in
    the user implementation and return more details in an enum.
    @return error state
  */
  /**************************************************************************/
  virtual int hasError()
  {
    return _error;
  }

protected:
  bool _enabled = false;
  bool _homed = false;
  bool _error = false;

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
  virtual void _reportMotionPoint() = 0;

  float _travel = 0.0;
  float _keepout = 0.0;
  float _maxPosition = 0.0;
  float _maxSpeed = 0.0;
  float _maxAcceleration = 0.0;
  bool _invertDirection = false;

  TickType_t _timeSliceInMs = 50;
  void (*_cbMotionPoint)(unsigned int, float, float, float, float) = NULL;
  HomingCallbackType _callBackHoming;
  static void _positionFeedbackTaskImpl(void *_this)
  {
    static_cast<MotorInterface *>(_this)->_positionFeedbackTask();
  }
  TaskHandle_t _taskPositionFeedbackHandle = NULL;
  void _positionFeedbackTask()
  {
    TickType_t xLastWakeTime;
    // Initialize the xLastWakeTime variable with the current tick count.
    xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
      // Return results of current motion point via the callback
      _reportMotionPoint();

      // Delay the task until the next tick count
      vTaskDelayUntil(&xLastWakeTime, _timeSliceInMs);
    }
  }
};
