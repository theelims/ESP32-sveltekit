/**
 *   Trapezoidal Profile Generator of StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 - 2024 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <Arduino.h>
#include <StrokeEngineTrapezoidalMotion.h>
#include <esp_timer.h>

TrapezoidalMotion::TrapezoidalMotion()
{
    resetProfileTo(0.0);
}

void TrapezoidalMotion::resetProfileTo(float position)
{
    if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
    {
        for (int i = 0; i < 5; i++)
        {
            _trapezoidalProfile[i].position = position;
            _trapezoidalProfile[i].speed = 0.0;
            _trapezoidalProfile[i].time = 0.0;
        }
        _acceleration = 0.0;
        _startOfProfileInMs = millis();
        _motionCompleted = true;

        ESP_LOGD("TrapezoidalMotion", "Reset trapezoidal profile to %05.1f mm", position);
        xSemaphoreGive(_parameterMutex);
    }
}

void TrapezoidalMotion::generateTrapezoidalProfile(float position, float speed, float acceleration)
{
    u_int64_t timestamp = esp_timer_get_time();
    u_int64_t sqrtTimestamp = esp_timer_get_time();
    float topSpeed = 0.0;
    float timeDelta = 0.0;

    ESP_LOGD("TrapezoidalMotion", "Calculate trapezoidal profile to %05.1f mm @ %05.1f mm/s, %05.1f mm/s^2", position, speed, acceleration);

    // Retrieve current speed and position
    unsigned int now = millis();
    speedAndPosition currentSpeedAndPosition = getSpeedAndPositionAbsolute(now);

    // Safeguard thread against race condition
    if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
    {
        // Reset profile debug messages index
        _profilePhaseDebugMessages = 0;

        ESP_LOGD("TrapezoidalMotion", "Current position is %05.1f mm @ %05.1f mm/s, %05.1f mm/s^2", currentSpeedAndPosition.position, currentSpeedAndPosition.speed, currentSpeedAndPosition.acceleration);

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
        _trapezoidalProfile[0].time = 0.0;
        _trapezoidalProfile[0].position = currentSpeedAndPosition.position;
        _trapezoidalProfile[0].speed = currentSpeedAndPosition.speed;

        ESP_LOGV("TrapezoidalMotion", "Ramp Point 0 is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalProfile[0].time, _trapezoidalProfile[0].position, _trapezoidalProfile[0].speed);

        // R A M P   P O I N T   1   - Do we need to decelerate?
        // Calculated deceleration to stand still --> also becomes all 0 if we are already at stand still.
        _trapezoidalProfile[1].time = abs(currentSpeedAndPosition.speed) / acceleration;
        _trapezoidalProfile[1].speed = 0.0;

        // Is a full stop requested? Then there is nothing to do after the deceleration to 0
        if (speed == 0.0)
        {
            for (int i = 2; i < 5; i++)
            {
                _trapezoidalProfile[i].time = _trapezoidalProfile[1].time;
                _trapezoidalProfile[i].position = _trapezoidalProfile[1].position;
                _trapezoidalProfile[i].speed = 0.0;
                ESP_LOGV("TrapezoidalMotion", "Ramp Points 1-4 all 0: Full Stop requested.");
            }
            // we are done, can give the mutex back and return
            xSemaphoreGive(_parameterMutex);
            return;
        }

        // calculate teh position at the end of the deceleration phase for a full stop
        if (currentSpeedAndPosition.speed < 0)
        {
            _trapezoidalProfile[1].position = currentSpeedAndPosition.position - 0.5 * acceleration * sq(_trapezoidalProfile[1].time);
        }
        else
        {
            _trapezoidalProfile[1].position = currentSpeedAndPosition.position + 0.5 * acceleration * sq(_trapezoidalProfile[1].time);
        }

        // Do we keep traveling in the same direction? Then we can decelerate to the new speed
        if (signbit(position - currentSpeedAndPosition.position) == signbit(currentSpeedAndPosition.speed))
        {
            ESP_LOGV("TrapezoidalMotion", "Still traveling in the same direction");
            // Will we overshoot? Standstill position > target position
            if (abs(position - _trapezoidalProfile[1].position) > abs(position - currentSpeedAndPosition.position))
            {
                // in that case we can decelerate to zero --> all values set correctly, already
                ESP_LOGV("TrapezoidalMotion", "But we will overshoot and reverse the direction to reach our target");
            }
            // will we need to slow down
            else if (abs(currentSpeedAndPosition.speed) > speed)
            {
                _trapezoidalProfile[1].time = (abs(currentSpeedAndPosition.speed) - speed) / acceleration;

                // traveling backwards
                if (currentSpeedAndPosition.speed < 0)
                {
                    _trapezoidalProfile[1].speed = -speed;
                    _trapezoidalProfile[1].position = currentSpeedAndPosition.position - 0.5 * acceleration * sq(_trapezoidalProfile[1].time) + currentSpeedAndPosition.speed * _trapezoidalProfile[1].time;
                }
                // traveling forwards
                else
                {
                    _trapezoidalProfile[1].speed = speed;
                    _trapezoidalProfile[1].position = currentSpeedAndPosition.position + 0.5 * acceleration * sq(_trapezoidalProfile[1].time) + currentSpeedAndPosition.speed * _trapezoidalProfile[1].time;
                }
                ESP_LOGV("TrapezoidalMotion", "Slowing down");
            }
            // then we must accelerate --> skip
            else
            {
                _trapezoidalProfile[1].time = _trapezoidalProfile[0].time;
                _trapezoidalProfile[1].position = _trapezoidalProfile[0].position;
                _trapezoidalProfile[1].speed = _trapezoidalProfile[0].speed;
                ESP_LOGV("TrapezoidalMotion", "No deceleration --> Skip");
            }
        }

        ESP_LOGV("TrapezoidalMotion", "Ramp Point 1 (Deceleration) is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalProfile[1].time, _trapezoidalProfile[1].position, _trapezoidalProfile[1].speed);

        // R A M P   P O I N T   2   - Do we need to accelerate?
        // Are we at coasting speed already? --> skip
        if (abs(_trapezoidalProfile[1].speed) == speed)
        {
            _trapezoidalProfile[2].time = _trapezoidalProfile[1].time;
            _trapezoidalProfile[2].position = _trapezoidalProfile[1].position;
            _trapezoidalProfile[2].speed = _trapezoidalProfile[1].speed;
            ESP_LOGV("TrapezoidalMotion", "Already coasting at the right speed --> Skip");
        }
        // We need to accelerate to coasting speed
        else
        {
            // Calculate triangular profile first to determine if acceleration is high enough for a trapezoidal profile
            // this calculates the time required for the acceleration phase of the triangle
            sqrtTimestamp = esp_timer_get_time();
            timeDelta = (1.41421 * sqrt(sq(_trapezoidalProfile[1].speed) + 2 * acceleration * abs(position - _trapezoidalProfile[1].position)) + 2 * _trapezoidalProfile[1].speed) / (2 * acceleration);
            _sqrtCalculationTime += esp_timer_get_time() - sqrtTimestamp;

            // Calculate the top speed of the triangle
            topSpeed = abs(_trapezoidalProfile[1].speed) + acceleration * timeDelta;

            // If top speed of triangle is higher then the commanded speed we do have a trapezoidal motion:
            ESP_LOGV("TrapezoidalMotion", "Calculate %s", (topSpeed <= speed) ? "triangular profile" : "trapezoidal profile");
            ESP_LOGV("TrapezoidalMotion", "Speed setpoint is %05.1f mm/s and triangular top speed %05.1f mm/s ", speed, topSpeed);
            if (topSpeed > speed)
            {
                // for a trapezoidal profile the next ramp time becomes
                timeDelta = (speed - abs(_trapezoidalProfile[1].speed)) / acceleration;

                // and the speed is as requested
                topSpeed = speed;
            }

            // traveling backwards
            if (position - _trapezoidalProfile[1].position < 0)
            {
                _trapezoidalProfile[2].speed = -topSpeed;
                _trapezoidalProfile[2].position = _trapezoidalProfile[1].position - 0.5 * acceleration * sq(timeDelta) + _trapezoidalProfile[1].speed * timeDelta;
            }
            // traveling forwards
            else
            {
                _trapezoidalProfile[2].speed = topSpeed;
                _trapezoidalProfile[2].position = _trapezoidalProfile[1].position + 0.5 * acceleration * sq(timeDelta) + _trapezoidalProfile[1].speed * timeDelta;
            }

            _trapezoidalProfile[2].time = _trapezoidalProfile[1].time + timeDelta;
        }

        ESP_LOGV("TrapezoidalMotion", "Ramp Point 2 (Acceleration) is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalProfile[2].time, _trapezoidalProfile[2].position, _trapezoidalProfile[2].speed);

        // R A M P   P O I N T   3   - Coasting at constant speed
        // If speed is not reached, we can skip as we are in a triangular profile
        if (abs(_trapezoidalProfile[2].speed) < speed)
        {
            _trapezoidalProfile[3].time = _trapezoidalProfile[2].time;
            _trapezoidalProfile[3].position = _trapezoidalProfile[2].position;
            ESP_LOGV("TrapezoidalMotion", "No coasting in a triangular profile --> skip");
        }
        // coasting until we hit the deceleration point
        else
        {
            timeDelta = abs(_trapezoidalProfile[2].speed) / acceleration;
            if (_trapezoidalProfile[2].speed < 0)
            {
                _trapezoidalProfile[3].position = position + 0.5 * acceleration * sq(timeDelta);
            }
            else
            {
                _trapezoidalProfile[3].position = position - 0.5 * acceleration * sq(timeDelta);
            }
            _trapezoidalProfile[3].time = _trapezoidalProfile[2].time + abs(_trapezoidalProfile[3].position - _trapezoidalProfile[2].position) / abs(_trapezoidalProfile[2].speed);
        }

        // speed is not affected by coasting
        _trapezoidalProfile[3].speed = _trapezoidalProfile[2].speed;

        ESP_LOGV("TrapezoidalMotion", "Ramp Point 3 (Coasting) is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalProfile[3].time, _trapezoidalProfile[3].position, _trapezoidalProfile[3].speed);

        // R A M P   P O I N T   4   - Deceleration to standstill
        _trapezoidalProfile[4].time = _trapezoidalProfile[3].time + abs(_trapezoidalProfile[3].speed) / acceleration;
        _trapezoidalProfile[4].position = position;
        _trapezoidalProfile[4].speed = 0.0;

        ESP_LOGV("TrapezoidalMotion", "Ramp Point 4 (Deceleration) is at %05.1f s @ %05.1f mm and %05.1f mm/s", _trapezoidalProfile[4].time, _trapezoidalProfile[4].position, _trapezoidalProfile[4].speed);

        xSemaphoreGive(_parameterMutex);
    }
    _profileGenerationTime += esp_timer_get_time() - timestamp;
    _profileGenerationCount++;
}

void TrapezoidalMotion::logProfilePerformance()
{
    if (_profileGenerationCount > 0)
    {
        ESP_LOGI("TrapezoidalMotion", "Avrg. Profile Generation Time: %d us", _profileGenerationTime / _profileGenerationCount);
        ESP_LOGI("TrapezoidalMotion", "Avrg. Sqrt Calculation Time: %d us", _sqrtCalculationTime / _profileGenerationCount);

        // Reset performance counters
        _profileGenerationTime = 0;
        _sqrtCalculationTime = 0;
        _profileGenerationCount = 0;
    }
}

speedAndPosition TrapezoidalMotion::getSpeedAndPositionAbsolute(unsigned int absoluteTimeInMs)
{
    return getSpeedAndPosition(absoluteTimeInMs - _startOfProfileInMs);
}

speedAndPosition TrapezoidalMotion::getSpeedAndPosition(unsigned int timeSinceStartInMs)
{
    speedAndPosition result;

    if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
    {
        // Calculate time base in Seconds
        float t = float(timeSinceStartInMs) * 1.0e-3;

        // Calculate return values based on ramp phase
        if (t < _trapezoidalProfile[1].time)
        {
            // Deceleration Phase
            if (_trapezoidalProfile[0].speed > 0)
            {
                result.speed = _trapezoidalProfile[0].speed - _acceleration * t;
                result.position = _trapezoidalProfile[0].position + _trapezoidalProfile[0].speed * t - 0.5 * _acceleration * sq(t);
            }
            else
            {
                result.speed = _trapezoidalProfile[0].speed + _acceleration * t;
                result.position = _trapezoidalProfile[0].position + _trapezoidalProfile[0].speed * t + 0.5 * _acceleration * sq(t);
            }
            // if (_profilePhaseDebugMessages < 1) {
            // ESP_LOGV("TrapezoidalMotion", "Phase 0: Deceleration");
            //     _profilePhaseDebugMessages = 1;
            // }
        }
        else if (t < _trapezoidalProfile[2].time)
        {
            // Acceleration Phase
            if (_trapezoidalProfile[2].speed > 0)
            {
                result.speed = _trapezoidalProfile[1].speed + _acceleration * (t - _trapezoidalProfile[1].time);
                result.position = _trapezoidalProfile[1].position + 0.5 * _acceleration * sq(t - _trapezoidalProfile[1].time);
            }
            else
            {
                result.speed = _trapezoidalProfile[1].speed - _acceleration * (t - _trapezoidalProfile[1].time);
                result.position = _trapezoidalProfile[1].position - 0.5 * _acceleration * sq(t - _trapezoidalProfile[1].time);
            }
            // if (_profilePhaseDebugMessages < 2) {
            // ESP_LOGV("TrapezoidalMotion", "Phase 1: Acceleration");
            //     _profilePhaseDebugMessages = 2;
            // }
        }
        else if (t < _trapezoidalProfile[3].time)
        {
            // Coasting Phase
            result.speed = _trapezoidalProfile[2].speed;
            result.position = _trapezoidalProfile[2].position + _trapezoidalProfile[2].speed * (t - _trapezoidalProfile[2].time);
            // if (_profilePhaseDebugMessages < 3) {
            // ESP_LOGV("TrapezoidalMotion", "Phase 2: Coasting");
            //     _profilePhaseDebugMessages = 3;
            // }
        }
        else if (t < _trapezoidalProfile[4].time)
        {
            // Deceleration Phase
            if (_trapezoidalProfile[3].speed > 0)
            {
                result.speed = _trapezoidalProfile[3].speed - _acceleration * (t - _trapezoidalProfile[3].time);
                result.position = _trapezoidalProfile[3].position + _trapezoidalProfile[3].speed * (t - _trapezoidalProfile[3].time) - 0.5 * _acceleration * sq(t - _trapezoidalProfile[3].time);
            }
            else
            {
                result.speed = _trapezoidalProfile[3].speed + _acceleration * (t - _trapezoidalProfile[3].time);
                result.position = _trapezoidalProfile[3].position + _trapezoidalProfile[3].speed * (t - _trapezoidalProfile[3].time) + 0.5 * _acceleration * sq(t - _trapezoidalProfile[3].time);
            }
            // if (_profilePhaseDebugMessages < 4) {
            // ESP_LOGV("TrapezoidalMotion", "Phase 3: Deceleration");
            //     _profilePhaseDebugMessages = 4;
            // }
        }
        else
        {
            result.speed = 0.0;
            result.position = _trapezoidalProfile[4].position;
            _motionCompleted = true;
            // if (_profilePhaseDebugMessages > 4) {
            // ESP_LOGV("TrapezoidalMotion", "Phase 4: Motion Complete");
            //     _profilePhaseDebugMessages = 5;
            // }
        }

        xSemaphoreGive(_parameterMutex);
    }
    return result;
}

float TrapezoidalMotion::getAcceleration()
{
    return _acceleration;
}

trapezoidalRampPoint TrapezoidalMotion::getProfilePoint(unsigned int index)
{
    unsigned int safeIndex = constrain(index, 0, 5);
    return _trapezoidalProfile[safeIndex];
}

unsigned int TrapezoidalMotion::getETA()
{
    return _trapezoidalProfile[5].time;
}

unsigned int TrapezoidalMotion::getETAAbsolute()
{
    return _trapezoidalProfile[5].time + _startOfProfileInMs;
}

bool TrapezoidalMotion::motionCompleted()
{
    // Call getSpeedAndPositionAbsolute to update the motionCompleted flag
    getSpeedAndPositionAbsolute(millis());
    return _motionCompleted;
}
