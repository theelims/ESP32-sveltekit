/**
 *   Virtual Motor Driver of StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef VIRTUAL_MOTOR_H
#define VIRTUAL_MOTOR_H

#include <StrokeEngineMotor.h>
#include <StrokeEngineTrapezoidalMotion.h>
#include <math.h>
#include <functional>

/**************************************************************************/
/*!
  @brief  The Virtual Motor inherits from MotorInterface and provides a
  purely virtual motor. It has a trapezoidal motion planner and returns the
  speed and position of StrokeEngine in real time. The time granularity is
  configurable. The motion planner mimics the one of the FastAccelStepper-
  library, allowing in-motion updates and recalculations. The main purpose
  for testing of StrokeEngine's safety features, new features and new patterns
  without putting real hardware at risk.
*/
/**************************************************************************/
class VirtualMotor : public MotorInterface
{
public:
  VirtualMotor() {}

  /**************************************************************************/
  /*!
    @brief  Initializes the virtual motor Arduino Style.
    @param timeInMs time interval at which speed and position should be
    reported in [ms]
  */
  /**************************************************************************/
  void begin()
  {
    // Since it is virtual no homing needed
    home();
  }

  /**************************************************************************/
  /*!
    @brief  A virtual home function. Since a virtual driver always knows where
    it is this can be used to reset the driver to 0.0mm at 0m/s velocity.
  */
  /**************************************************************************/
  void home()
  {
    _homed = true;
    ESP_LOGI("VirtualMotor", "Motor homed!");

    _trapezoidalProfile.resetProfileTo(0.0);

    if (_callBackHoming != NULL)
      _callBackHoming();
  }

  /**************************************************************************/
  /*!
    @brief  Can be used to change the update interval.
    @param timeInMs time interval at which speed and position should be
    reported in [ms]
  */
  /**************************************************************************/
  void setTimeGranularity(unsigned int timeInMs) { _timeSliceInMs = timeInMs / portTICK_PERIOD_MS; }

  /**************************************************************************/
  /*!
    @brief  Enables the motor driver. This starts the task reporting speed and
    position at the specified intervals.
  */
  /**************************************************************************/
  void enable()
  {
    ESP_LOGI("VirtualMotor", "Motor Enabled!");
    _enabled = true;

    // Reset motion to home position
    home();

    // Create / resume motion simulator task
    if (_taskMotionSimulatorHandle == NULL)
    {
      // Create Stroke Task
      xTaskCreatePinnedToCore(
          _motionSimulatorTaskImpl,    // Function that should be called
          "Motion Simulation",         // Name of the task (for debugging)
          4096,                        // Stack size (bytes)
          this,                        // Pass reference to this class instance
          10,                          // Pretty high task priority
          &_taskMotionSimulatorHandle, // Task handle
          1                            // Pin to application core
      );
      ESP_LOGD("VirtualMotor", "Created Position Feedback Task.");
    }
    else
    {
      // Resume task, if it already exists
      vTaskResume(_taskMotionSimulatorHandle);
      ESP_LOGD("VirtualMotor", "Resumed Position Feedback Task.");
    }
  }

  /**************************************************************************/
  /*!
    @brief  Disables the motor driver. This stops the task reporting speed and
    position.
  */
  /**************************************************************************/
  void disable()
  {
    ESP_LOGI("VirtualMotor", "Stepper Disabled!");
    _enabled = false;

    _trapezoidalProfile.resetProfileTo(getPosition());

    // Suspend motion simulator task if it exists already
    if (_taskMotionSimulatorHandle != NULL)
    {
      vTaskSuspend(_taskMotionSimulatorHandle);
      ESP_LOGD("VirtualMotor", "Suspended Position Feedback Task.");
    }
  };

  /**************************************************************************/
  /*!
    @brief  Initiates the fastest safe breaking to stand-still stopping all
    motion without loosing position.
  */
  /**************************************************************************/
  void stopMotion()
  {
    ESP_LOGW("VirtualMotor", "STOP MOTION!");
    _unsafeGoToPosition(0.0, 0.0, _maxAcceleration);
  }

  /**************************************************************************/
  /*!
    @brief  Returns if a trapezoidal motion is carried out, or the machine is
    at stand-still.
    @return `true` if motion is completed, `false` if still under way
  */
  /**************************************************************************/
  bool motionCompleted()
  {
    return _trapezoidalProfile.motionCompleted();
  }

  /**************************************************************************/
  /*!
    @brief  Returns the currently used acceleration.
    @return acceleration of the motor in [mm/s²]
  */
  /**************************************************************************/
  float getAcceleration() { return _trapezoidalProfile.getAcceleration(); }

  /**************************************************************************/
  /*!
    @brief  Returns the current speed the machine.
    @return speed of the motor in [mm/s]
  */
  /**************************************************************************/
  float getSpeed() { return _trapezoidalProfile.getSpeedAndPositionAbsolute(millis()).speed; }

  /**************************************************************************/
  /*!
    @brief  Returns the current position of the machine.
    @return position in [mm]
  */
  /**************************************************************************/
  float getPosition() { return _trapezoidalProfile.getSpeedAndPositionAbsolute(millis()).position; }

protected:
  void _unsafeGoToPosition(float position, float speed, float acceleration)
  {
    _trapezoidalProfile.generateTrapezoidalProfile(position, speed, acceleration);
  }

private:
  TrapezoidalMotion _trapezoidalProfile;
  TickType_t _timeSliceInMs = 50;
  void _reportMotionPoint()
  { // Delete task, as it is not needed for the virtual motor
    vTaskDelete(NULL);
  }
  static void _motionSimulatorTaskImpl(void *_this) { static_cast<VirtualMotor *>(_this)->_motionSimulatorTask(); }
  void _motionSimulatorTask()
  {
    TickType_t xLastWakeTime;
    // Initialize the xLastWakeTime variable with the current tick count.
    xLastWakeTime = xTaskGetTickCount();

    unsigned int now = millis();
    speedAndPosition currentSpeedAndPosition;

    while (true)
    {
      // Establish time stamp
      now = millis();

      // calculate current speed and position
      currentSpeedAndPosition = _trapezoidalProfile.getSpeedAndPositionAbsolute(now);

      // Return results of current motion point via the callback
      if (_cbMotionPoint != NULL)
        _cbMotionPoint(now, currentSpeedAndPosition.position, currentSpeedAndPosition.speed, 0.0, 0.0);

      // Delay the task until the next tick count
      vTaskDelayUntil(&xLastWakeTime, _timeSliceInMs);
    }
  }
  TaskHandle_t _taskMotionSimulatorHandle = NULL;
};

#endif // VIRTUAL_MOTOR_H
