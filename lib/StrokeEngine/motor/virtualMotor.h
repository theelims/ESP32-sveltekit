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

#pragma once

#include "motor.h"
#include "math.h"

/**************************************************************************/
/*!
  @brief  Struct defining a speed and position tuple
*/
/**************************************************************************/
struct speedAndPosition
{
    float speed;        /*> Speed in [mm/s] */
    float position;     /*> Position in [mm] */
    float acceleration; /*> Acceleration in [mm/s²] */
};

/**************************************************************************/
/*!
  @brief  Struct defining a point of a trapezoidal motion profile
*/
/**************************************************************************/
struct trapezoidalRampPoint
{
    float time;     /*> Time in [s] */
    float position; /*> Position in [mm] */
    float speed;    /*> Speed in [mm/s] */
};

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
      @brief  Initializes the virtual motor Arduino Style. It also attaches a
      callback function where the speed and position are reported on a regular
      interval specified with timeInMs.
      @param cbMotionPoint Callback with the signature
      `cbMotionPoint(unsigned int timestamp, float position, float speed)`. time is reported
      milliseconds since the controller has started (`millis()`), speed in [m/s] and
      position in [mm].
      @param timeInMs time interval at which speed and position should be
      reported in [ms]
    */
    /**************************************************************************/
    void begin(void (*cbMotionPoint)(unsigned int, float, float), unsigned int timeInMs)
    {
        _cbMotionPoint = cbMotionPoint;
        _timeSliceInMs = timeInMs / portTICK_PERIOD_MS;
        // Since it is virtual no homing needed
        home();

        // Set everything to defaults
        _acceleration = 0.0;
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
        // Safeguard thread against race condition
        if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
        {
            // initialize ramp with 0 so that system is at home position and in stand still
            for (int i = 0; i < 5; i++)
            {
                _trapezoidalRamp[i].position = 0.0;
                _trapezoidalRamp[i].speed = 0.0;
                _trapezoidalRamp[i].time = 0.0;
            }
            xSemaphoreGive(_parameterMutex);
        }
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

        if (_cbMotionPoint == NULL)
        {
            ESP_LOGE("VirtualMotor", "Could not create Position Feedback Task! Please provide callback function.");
            return;
        }

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
        if (_timeSliceInMs >= 50)
        {
            _currentSpeedAndPosition(millis());
        }
        return _motionCompleted;
    }

    /**************************************************************************/
    /*!
      @brief  Returns the currently used acceleration.
      @return acceleration of the motor in [mm/s²]
    */
    /**************************************************************************/
    float getAcceleration() { return _acceleration; }

    /**************************************************************************/
    /*!
      @brief  Returns the current speed the machine.
      @return speed of the motor in [mm/s]
    */
    /**************************************************************************/
    float getSpeed() { return _currentSpeedAndPosition(millis()).speed; }

    /**************************************************************************/
    /*!
      @brief  Returns the current position of the machine.
      @return position in [mm]
    */
    /**************************************************************************/
    float getPosition() { return _currentSpeedAndPosition(millis()).position; }

protected:
    void _unsafeGoToPosition(float position, float speed, float acceleration)
    {
        _motionCompleted = false;
        _trapezoidalRampGenerator(position, speed, acceleration);
    }

private:
    void (*_cbMotionPoint)(unsigned int, float, float) = NULL;
    TickType_t _timeSliceInMs = 50;
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

            // Safeguard thread against race condition
            if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
            {
                // Establish time stamp
                now = millis();

                // calculate current speed and position
                currentSpeedAndPosition = _currentSpeedAndPosition(now);

                xSemaphoreGive(_parameterMutex);
            }

            // Return results of current motion point via the callback
            _cbMotionPoint(now, currentSpeedAndPosition.position, currentSpeedAndPosition.speed);

            // Delay the task until the next tick count
            vTaskDelayUntil(&xLastWakeTime, _timeSliceInMs);
        }
    }
    TaskHandle_t _taskMotionSimulatorHandle = NULL;
    float _acceleration = 0.0;
    unsigned int _startOfProfileInMs = 0;
    bool _motionCompleted = true;
    trapezoidalRampPoint _trapezoidalRamp[5];
    SemaphoreHandle_t _parameterMutex = xSemaphoreCreateMutex();
    int _profilePhaseDebugMessages = 0;
    void _trapezoidalRampGenerator(float position, float speed, float acceleration)
    {

        float topSpeed = 0.0;
        float timeDelta = 0.0;

        ESP_LOGD("VirtualMotor", "Calculate trapezoidal profile to %05.1f mm @ %05.1f mm/s, %05.1f mm/s^2", position, speed, acceleration);

        // Safeguard thread against race condition
        if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
        {
            // Reset profile debug messages index
            _profilePhaseDebugMessages = 0;

            // Retrieve current speed and position
            unsigned int now = millis();
            speedAndPosition currentSpeedAndPosition = _currentSpeedAndPosition(now);
            ESP_LOGD("VirtualMotor", "Current position is %05.1f mm @ %05.1f mm/s, %05.1f mm/s^2", currentSpeedAndPosition.position, currentSpeedAndPosition.speed, currentSpeedAndPosition.acceleration);

            // Save time as basis for later calculations
            _startOfProfileInMs = now;

            // Flag in-motion
            _motionCompleted = false;

            // store motion defining parameters
            _acceleration = acceleration;

            // The motion generator may be called while in motion and starts the ramp calculation with the current speed and position
            // In this case a trapezoidal motion always consists of these phases:
            // Now --[0]--> Deceleration --[1]--> Acceleration --[2]--> Coasting --[3]--> Deceleration to zero --[4]--> stand still / motion completed
            // Depending on the conditions certain phases have the time=0 and are effectively skipped.

            // R A M P   P O I N T   0   - Where everything starts
            _trapezoidalRamp[0].time = 0.0;
            _trapezoidalRamp[0].position = currentSpeedAndPosition.position;
            _trapezoidalRamp[0].speed = currentSpeedAndPosition.speed;

            ESP_LOGD("VirtualMotor", "Ramp Point 0 is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalRamp[0].time, _trapezoidalRamp[0].position, _trapezoidalRamp[0].speed);

            // R A M P   P O I N T   1   - Do we need to decelerate?
            // Calculated deceleration to stand still --> also becomes all 0 if we are already at stand still.
            _trapezoidalRamp[1].time = abs(currentSpeedAndPosition.speed) / acceleration;
            _trapezoidalRamp[1].speed = 0.0;
            if (currentSpeedAndPosition.speed < 0)
            {
                _trapezoidalRamp[1].position = currentSpeedAndPosition.position - 0.5 * acceleration * sq(_trapezoidalRamp[1].time);
            }
            else
            {
                _trapezoidalRamp[1].position = currentSpeedAndPosition.position + 0.5 * acceleration * sq(_trapezoidalRamp[1].time);
            }

            // Is a full stop requested? Then there is nothing to do after the deceleration to 0
            if (speed == 0.0)
            {
                for (int i = 2; i < 5; i++)
                {
                    _trapezoidalRamp[i].time = _trapezoidalRamp[1].time;
                    _trapezoidalRamp[i].position = _trapezoidalRamp[1].position;
                    _trapezoidalRamp[i].speed = 0.0;
                    ESP_LOGD("VirtualMotor", "Ramp Points 1-4 all 0: Full Stop requested.");
                }
                // we are done, can give the mutex back and return
                xSemaphoreGive(_parameterMutex);
                return;
            }

            // Do we still travel in the same direction?
            if (signbit(position - currentSpeedAndPosition.position) == signbit(currentSpeedAndPosition.speed))
            {
                ESP_LOGD("VirtualMotor", "Still traveling in the same direction");
                // Will we overshoot? Standstill position > target position
                if (abs(position - _trapezoidalRamp[1].position) > abs(position - currentSpeedAndPosition.position))
                {
                    // in that case we can decelerate to zero --> all values set correctly, already
                    ESP_LOGD("VirtualMotor", "But we will overshoot and reverse the direction to reach our target");
                    // will we need to slow down
                }
                else if (abs(currentSpeedAndPosition.speed) > speed)
                {
                    _trapezoidalRamp[1].time = abs(currentSpeedAndPosition.speed) - speed / acceleration;

                    // traveling backwards
                    if (currentSpeedAndPosition.speed < 0)
                    {
                        _trapezoidalRamp[1].speed = -speed;
                        _trapezoidalRamp[1].position = currentSpeedAndPosition.position - 0.5 * acceleration * sq(_trapezoidalRamp[1].time) + currentSpeedAndPosition.speed * _trapezoidalRamp[1].time;

                        // traveling forwards
                    }
                    else
                    {
                        _trapezoidalRamp[1].speed = speed;
                        _trapezoidalRamp[1].position = currentSpeedAndPosition.position + 0.5 * acceleration * sq(_trapezoidalRamp[1].time) + currentSpeedAndPosition.speed * _trapezoidalRamp[1].time;
                    }
                    ESP_LOGD("VirtualMotor", "Slowing down");

                    // then we must accelerate --> skip
                }
                else
                {
                    _trapezoidalRamp[1].time = _trapezoidalRamp[0].time;
                    _trapezoidalRamp[1].position = _trapezoidalRamp[0].position;
                    _trapezoidalRamp[1].speed = _trapezoidalRamp[0].speed;
                    ESP_LOGD("VirtualMotor", "No deceleration --> Skip");
                }
            }

            ESP_LOGD("VirtualMotor", "Ramp Point 1 (Deceleration) is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalRamp[1].time, _trapezoidalRamp[1].position, _trapezoidalRamp[1].speed);

            // R A M P   P O I N T   2   - Do we need to accelerate?
            // Are we at coasting speed already? --> skip
            if (abs(_trapezoidalRamp[1].speed) == speed)
            {
                _trapezoidalRamp[2].time = _trapezoidalRamp[1].time;
                _trapezoidalRamp[2].position = _trapezoidalRamp[1].position;
                _trapezoidalRamp[2].speed = _trapezoidalRamp[1].speed;
                ESP_LOGD("VirtualMotor", "Already coasting at the right speed --> Skip");

                // We need to accelerate to coasting speed
            }
            else
            {
                // Calculate triangular profile first to determine if acceleration is high enough for a trapezoidal profile
                // this calculates the time required for the acceleration phase of the triangle
                timeDelta = (sqrt(2) * sqrt(sq(_trapezoidalRamp[1].speed) + 2 * acceleration * abs(position - _trapezoidalRamp[1].position)) + 2 * _trapezoidalRamp[1].speed) / (2 * acceleration);

                // Calculate the top speed of the triangle
                topSpeed = abs(_trapezoidalRamp[1].speed) + acceleration * timeDelta;

                // If top speed of triangle is higher then the commanded speed we do have a trapezoidal motion:
                ESP_LOGD("VirtualMotor", "Calculate %s", (topSpeed <= speed) ? "triangular profile" : "trapezoidal profile");
                ESP_LOGV("VirtualMotor", "Speed setpoint is %05.1f mm/s and triangular top speed %05.1f mm/s ", speed, topSpeed);
                if (topSpeed > speed)
                {
                    // for a trapezoidal profile the next ramp time becomes
                    timeDelta = (speed - abs(_trapezoidalRamp[1].speed)) / acceleration;

                    // and the speed is as requested
                    topSpeed = speed;
                }

                // traveling backwards
                if (position - _trapezoidalRamp[1].position < 0)
                {
                    _trapezoidalRamp[2].speed = -topSpeed;
                    _trapezoidalRamp[2].position = _trapezoidalRamp[1].position - 0.5 * acceleration * sq(timeDelta) + _trapezoidalRamp[1].speed * timeDelta;

                    // traveling forwards
                }
                else
                {
                    _trapezoidalRamp[2].speed = topSpeed;
                    _trapezoidalRamp[2].position = _trapezoidalRamp[1].position + 0.5 * acceleration * sq(timeDelta) + _trapezoidalRamp[1].speed * timeDelta;
                }

                _trapezoidalRamp[2].time = _trapezoidalRamp[1].time + timeDelta;
            }

            ESP_LOGD("VirtualMotor", "Ramp Point 2 (Acceleration) is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalRamp[2].time, _trapezoidalRamp[2].position, _trapezoidalRamp[2].speed);

            // R A M P   P O I N T   3   - Coasting at constant speed
            // If speed is not reached, we can skip as we are in a triangular profile
            if (abs(_trapezoidalRamp[2].speed) < speed)
            {
                _trapezoidalRamp[3].time = _trapezoidalRamp[2].time;
                _trapezoidalRamp[3].position = _trapezoidalRamp[2].position;
                ESP_LOGD("VirtualMotor", "No coasting in a triangular profile --> skip");

                // coasting until we hit the deceleration point
            }
            else
            {
                timeDelta = abs(_trapezoidalRamp[2].speed) / acceleration;
                if (_trapezoidalRamp[2].speed < 0)
                {
                    _trapezoidalRamp[3].position = position + 0.5 * acceleration * sq(timeDelta);
                }
                else
                {
                    _trapezoidalRamp[3].position = position - 0.5 * acceleration * sq(timeDelta);
                }
                _trapezoidalRamp[3].time = _trapezoidalRamp[2].time + abs(_trapezoidalRamp[3].position - _trapezoidalRamp[2].position) / abs(_trapezoidalRamp[2].speed);
            }

            // speed is not affected by coasting
            _trapezoidalRamp[3].speed = _trapezoidalRamp[2].speed;

            ESP_LOGD("VirtualMotor", "Ramp Point 3 (Coasting) is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalRamp[3].time, _trapezoidalRamp[3].position, _trapezoidalRamp[3].speed);

            // R A M P   P O I N T   4   - Deceleration to standstill
            _trapezoidalRamp[4].time = _trapezoidalRamp[3].time + abs(_trapezoidalRamp[3].speed) / acceleration;
            _trapezoidalRamp[4].position = position;
            _trapezoidalRamp[4].speed = 0.0;

            ESP_LOGD("VirtualMotor", "Ramp Point 4 (Deceleration) is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalRamp[4].time, _trapezoidalRamp[4].position, _trapezoidalRamp[4].speed);

            xSemaphoreGive(_parameterMutex);
        }
    }
    // This function must be called from within a xSemaphoreTake(_parameterMutex) == true scope
    speedAndPosition _currentSpeedAndPosition(unsigned int timeInMs)
    {
        speedAndPosition result;

        // Calculate time base in Seconds
        float t = float(timeInMs - _startOfProfileInMs) * 1.0e-3;

        // Calculate return values based on ramp phase
        if (t < _trapezoidalRamp[1].time)
        {
            // Deceleration Phase
            if (_trapezoidalRamp[0].speed > 0)
            {
                result.speed = _trapezoidalRamp[0].speed - _acceleration * t;
                result.position = _trapezoidalRamp[0].position + _trapezoidalRamp[0].speed * t - 0.5 * _acceleration * sq(t);
            }
            else
            {
                result.speed = _trapezoidalRamp[0].speed + _acceleration * t;
                result.position = _trapezoidalRamp[0].position + _trapezoidalRamp[0].speed * t + 0.5 * _acceleration * sq(t);
            }
            // if (_profilePhaseDebugMessages < 1) {
            ESP_LOGV("VirtualMotor", "Phase 0: Deceleration");
            //     _profilePhaseDebugMessages = 1;
            // }
        }
        else if (t < _trapezoidalRamp[2].time)
        {
            // Acceleration Phase
            if (_trapezoidalRamp[2].speed > 0)
            {
                result.speed = _trapezoidalRamp[1].speed + _acceleration * (t - _trapezoidalRamp[1].time);
                result.position = _trapezoidalRamp[1].position + 0.5 * _acceleration * sq(t - _trapezoidalRamp[1].time);
            }
            else
            {
                result.speed = _trapezoidalRamp[1].speed - _acceleration * (t - _trapezoidalRamp[1].time);
                result.position = _trapezoidalRamp[1].position - 0.5 * _acceleration * sq(t - _trapezoidalRamp[1].time);
            }
            // if (_profilePhaseDebugMessages < 2) {
            ESP_LOGV("VirtualMotor", "Phase 1: Acceleration");
            //     _profilePhaseDebugMessages = 2;
            // }
        }
        else if (t < _trapezoidalRamp[3].time)
        {
            // Coasting Phase
            result.speed = _trapezoidalRamp[2].speed;
            result.position = _trapezoidalRamp[2].position + _trapezoidalRamp[2].speed * (t - _trapezoidalRamp[2].time);
            // if (_profilePhaseDebugMessages < 3) {
            ESP_LOGV("VirtualMotor", "Phase 2: Coasting");
            //     _profilePhaseDebugMessages = 3;
            // }
        }
        else if (t < _trapezoidalRamp[4].time)
        {
            // Deceleration Phase
            if (_trapezoidalRamp[3].speed > 0)
            {
                result.speed = _trapezoidalRamp[3].speed - _acceleration * (t - _trapezoidalRamp[3].time);
                result.position = _trapezoidalRamp[3].position + _trapezoidalRamp[3].speed * (t - _trapezoidalRamp[3].time) - 0.5 * _acceleration * sq(t - _trapezoidalRamp[3].time);
            }
            else
            {
                result.speed = _trapezoidalRamp[3].speed + _acceleration * (t - _trapezoidalRamp[3].time);
                result.position = _trapezoidalRamp[3].position + _trapezoidalRamp[3].speed * (t - _trapezoidalRamp[3].time) + 0.5 * _acceleration * sq(t - _trapezoidalRamp[3].time);
            }
            // if (_profilePhaseDebugMessages < 4) {
            ESP_LOGV("VirtualMotor", "Phase 3: Deceleration");
            //     _profilePhaseDebugMessages = 4;
            // }
        }
        else
        {
            result.speed = 0.0;
            result.position = _trapezoidalRamp[4].position;
            _motionCompleted = true;
            // if (_profilePhaseDebugMessages > 4) {
            ESP_LOGV("VirtualMotor", "Phase 4: Motion Complete");
            //     _profilePhaseDebugMessages = 5;
            // }
        }

        return result;
    }
};
