/**
 *   StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <StrokeEngineSafeGuard.h>

void StrokeEngineSafeGuard::begin(MotorInterface *motor, float depth, float stroke, float rate, float depthLimit, float strokeLimit, float rateLimit, float velocityLimit, float easeInSpeed)
{
    _motor = motor,

    _rateLimit = constrain(rateLimit, RATE_LOWER_LIMIT, RATE_UPPER_LIMIT);
    _timeOfStrokeLimit = 60.0 / _rateLimit;
    _depthLimit = constrain(depthLimit, 0.0, _motor->getMaxPosition());
    _strokeLimit = constrain(strokeLimit, 0.0, _motor->getMaxPosition());
    _velocityLimit = constrain(velocityLimit, 0.0, _motor->getMaxSpeed());

    _depth = constrain(depth, 0.0, _depthLimit);
    _stroke = constrain(stroke, 0.0, _strokeLimit);
    _rate = constrain(rate, 0.0, _rateLimit);
    _timeOfStroke = 60.0 / _rate;

    _easeInSpeed = constrain(easeInSpeed, 0.0, _velocityLimit);

    ESP_LOGD("StrokeEngineSafeGuard", "Stroke Parameter Depth = %.2f", _depth);
    ESP_LOGD("StrokeEngineSafeGuard", "Stroke Parameter Depth Limit = %.2f", _depthLimit);
    ESP_LOGD("StrokeEngineSafeGuard", "Stroke Parameter Stroke = %.2f", _stroke);
    ESP_LOGD("StrokeEngineSafeGuard", "Stroke Parameter Stroke Limit = %.2f", _strokeLimit);
    ESP_LOGD("StrokeEngineSafeGuard", "Stroke Parameter Time of Stroke = %.2f", _timeOfStroke);
}

float StrokeEngineSafeGuard::setDepth(float depth)
{
    _depth = constrain(depth, 0.0, _depthLimit);
    ESP_LOGD("StrokeEngineSafeGuard", "Set Stroke Parameter Depth = %.2f", _depth);
    return _depth;
}

float StrokeEngineSafeGuard::getDepth()
{
    return _depth;
}

float StrokeEngineSafeGuard::setStroke(float stroke)
{
    _stroke = constrain(stroke, 0.0, _strokeLimit);
    ESP_LOGD("StrokeEngineSafeGuard", "Set Stroke Parameter Stroke = %.2f", _stroke);
    return _stroke;
}

float StrokeEngineSafeGuard::getStroke()
{
    return _stroke;
}

float StrokeEngineSafeGuard::setRate(float rate)
{
    _rate = constrain(rate, 0.0, _rateLimit);
    _timeOfStroke = 60.0 / _rate;
    ESP_LOGD("StrokeEngineSafeGuard", "Set Stroke Parameter Rate = %.2f -> ToS = %.2f", _rate, _timeOfStroke);
    return _rate;
}

float StrokeEngineSafeGuard::getRate()
{
    return _rate;
}

float StrokeEngineSafeGuard::getTimeOfStroke()
{
    return _timeOfStroke;
}

float StrokeEngineSafeGuard::setDepthLimit(float depthLimit)
{
    _depthLimit = constrain(depthLimit, 0.0, _motor->getMaxPosition());
    _depth = constrain(_depth, 0.0, _depthLimit);
    ESP_LOGD("StrokeEngineSafeGuard", "Set Safety Parameter Depth Limit = %.2f", _depthLimit);
    return _depthLimit;
}

float StrokeEngineSafeGuard::getDepthLimit()
{
    return _depthLimit;
}

float StrokeEngineSafeGuard::setStrokeLimit(float strokeLimit)
{
    _strokeLimit = constrain(strokeLimit, 0.0, _motor->getMaxPosition());
    _stroke = constrain(_stroke, 0.0, _strokeLimit);
    ESP_LOGD("StrokeEngineSafeGuard", "Set Safety Parameter Stroke Limit = %.2f", _strokeLimit);
    return _strokeLimit;
}

float StrokeEngineSafeGuard::getStrokeLimit()
{
    return _strokeLimit;
}

float StrokeEngineSafeGuard::setRateLimit(float rateLimit)
{
    _rateLimit = constrain(rateLimit, RATE_LOWER_LIMIT, RATE_UPPER_LIMIT);
    _timeOfStrokeLimit = 60.0 / _rateLimit;
    _rate = constrain(_rate, 0.0, _rateLimit);
    _timeOfStroke = 60.0 / _rate;
    ESP_LOGD("StrokeEngineSafeGuard", "Set Safety Parameter Rate Limit = %.2f -> ToS = %.2f", _rateLimit, _timeOfStrokeLimit);
    return _rateLimit;
}

float StrokeEngineSafeGuard::getRateLimit()
{
    return _rateLimit;
}

float StrokeEngineSafeGuard::getTimeOfStrokeLimit()
{
    return _timeOfStrokeLimit;
}

float StrokeEngineSafeGuard::setVelocityLimit(float velocityLimit)
{
    _velocityLimit = constrain(velocityLimit, 0.0, _motor->getMaxSpeed());
    _easeInSpeed = constrain(_easeInSpeed, 0.0, _velocityLimit);
    ESP_LOGD("StrokeEngineSafeGuard", "Set Safety Parameter Velocity Limit = %.2f", _velocityLimit);
    return _velocityLimit;
}

float StrokeEngineSafeGuard::getVelocityLimit()
{
    return _velocityLimit;
}

float StrokeEngineSafeGuard::setEaseInSpeed(float easeInSpeed)
{
    _easeInSpeed = constrain(easeInSpeed, 0.0, _velocityLimit);
    ESP_LOGD("StrokeEngineSafeGuard", "Set Safety Parameter Ease In Speed = %.2f", _easeInSpeed);
    return _easeInSpeed;
}

float StrokeEngineSafeGuard::getEaseInSpeed()
{
    return _easeInSpeed;
}

SafeStrokeParameters_t StrokeEngineSafeGuard::makeSafe(motionParameters_t motionParameters)
{
    return makeSafe(motionParameters.stroke, motionParameters.speed, motionParameters.acceleration);
}

SafeStrokeParameters_t StrokeEngineSafeGuard::makeSafeRelative(float relativeTargetPosition, float speed, float acceleration)
{
    float stroke = constrain(relativeTargetPosition, 0.0, 1.0) * _stroke;
    return makeSafe(stroke, speed, acceleration);
}

SafeStrokeParameters_t StrokeEngineSafeGuard::makeSafe(float stroke, float speed, float acceleration)
{
    SafeStrokeParameters_t safeStrokeParameters;

    // Calculate the absolute target position
    // Constraint the stroke and offset by depth
    safeStrokeParameters.absoluteTargetPosition = (_depth - _stroke) + constrain(stroke, 0.0, _stroke);

    // Calculate speed
    // Constrain speed to ensure it obeys to motion boundaries
    safeStrokeParameters.speed = constrain(speed, 0.0, _velocityLimit);

    // Calculate acceleration
    // Crash avoidance on on-the-fly parameter update
    if (_motor->motionCompleted() == false && _motor->getAcceleration() > acceleration)
    {
        ESP_LOGW("StrokeEngineSafeGuard", "Crash avoidance! Set Acceleration from %05.1f to %05.1f", acceleration, _motor->getAcceleration());
        safeStrokeParameters.acceleration = _motor->getAcceleration();
    }
    else
    {
        // Constrain acceleration to ensure it obeys to motion boundaries
        safeStrokeParameters.acceleration = constrain(acceleration, 0.0, _motor->getMaxAcceleration());
    }

    // ESP_LOGV("StrokeEngineSafeGuard", "Safe Stroke Parameters: Absolute Target Position = %.2f, Speed = %.2f, Acceleration = %.2f", safeStrokeParameters.absoluteTargetPosition, safeStrokeParameters.speed, safeStrokeParameters.acceleration);

    return safeStrokeParameters;
}
