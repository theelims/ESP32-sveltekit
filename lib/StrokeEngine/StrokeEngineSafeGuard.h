/**
 *   StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2024 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef STROKE_ENGINE_SAFE_GUARD_H
#define STROKE_ENGINE_SAFE_GUARD_H

#include <StrokeEnginePattern.h>
#include <patternMath.h>
#include <StrokeEngineMotor.h>

#ifndef RATE_LOWER_LIMIT
#define RATE_LOWER_LIMIT 0.5
#endif

#ifndef RATE_UPPER_LIMIT
#define RATE_UPPER_LIMIT 600.0
#endif

#ifndef EASE_IN_UPDATE_INTERVAL
#define EASE_IN_UPDATE_INTERVAL 50
#endif

typedef struct
{
    float absoluteTargetPosition;
    float strokeLength;
    float speed;
    float acceleration;
} SafeStrokeParameters_t;

class StrokeEngineSafeGuard
{
public:
    void begin(MotorInterface *motor, float depth, float stroke, float rate, float depthLimit, float strokeLimit, float rateLimit, float velocityLimit, float easeInSpeed);

    float setDepth(float depth);
    float getCurrentDepth();
    float getTargetDepth();

    float setStroke(float stroke);
    float getCurrentStroke();
    float getTargetStroke();

    float setRate(float rate);
    float getRate();
    float getTimeOfStroke();

    float setDepthLimit(float depthLimit);
    float getDepthLimit();

    float setStrokeLimit(float strokeLimit);
    float getStrokeLimit();

    float setRateLimit(float rateLimit);
    float getRateLimit();
    float getTimeOfStrokeLimit();

    float setVelocityLimit(float velocityLimit);
    float getVelocityLimit();

    float setEaseInSpeed(float easeInSpeed);
    float getEaseInSpeed();

    bool calculateEaseIn(); // Check if the current motion needs to be eased in and update _currentDepth and _currentStroke

    SafeStrokeParameters_t makeSafe(float stroke, float speed, float acceleration);
    SafeStrokeParameters_t makeSafe(motionParameters_t motionParameters);
    SafeStrokeParameters_t makeSafeRelative(float relativeTargetPosition, float speed, float acceleration);

protected:
    MotorInterface *_motor = nullptr;
    float _targetDepth;
    float _currentDepth;
    float _targetStroke;
    float _currentStroke;
    float _rate;
    float _timeOfStroke;

    float _depthLimit;
    float _strokeLimit;
    float _rateLimit;
    float _timeOfStrokeLimit;
    float _velocityLimit;

    float _easeInSpeed;

    unsigned int _lastEaseInCalculation = 0;
};

#endif // STROKE_ENGINE_SAFE_GUARD_H