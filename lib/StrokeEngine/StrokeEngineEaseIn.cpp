/**
 *   Ease In Motion Profile Modifier of StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 - 2024 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <StrokeEngineEaseIn.h>

EaseInModifier::EaseInModifier(TrapezoidalMotion *profile, StrokeEngineSafeGuard *safeGuard) : _profile(profile), _safeGuard(safeGuard)
{
}

void EaseInModifier::begin()
{
    _easeInSpeed = _safeGuard->getEaseInSpeed();
    _depth = _safeGuard->getDepth();
    _previousDepth = _depth;
    _depthChangeTime = 0;
    _depthETA = 0;
    _stroke = _safeGuard->getStroke();
    _previousStroke = _stroke;
    _strokeChangeTime = 0;
    _strokeETA = 0;
    ESP_LOGD("EaseInModifier", "Begin EaseIn with Speed = %.2f", _easeInSpeed);
}

void EaseInModifier::setDepth(float depth)
{
    _previousDepth = _depth;
    _depth = depth;
    _depthChangeTime = millis();
    unsigned int dt = (unsigned int)(abs(_depth - _previousDepth) / _easeInSpeed * 1000.0);
    _depthETA = _depthChangeTime + dt;
    ;
    ESP_LOGD("EaseInModifier", "Set Depth = %.2f, dt = %lums, ETA = %lu", _depth, dt, _depthETA);
}

void EaseInModifier::setStroke(float stroke)
{
    _previousStroke = _stroke;
    _stroke = stroke;
    _strokeChangeTime = millis();
    unsigned int dt = (unsigned int)((_stroke - _previousStroke) / _easeInSpeed * 1000.0);
    _strokeETA = _strokeChangeTime + dt;
    ESP_LOGD("EaseInModifier", "Set Stroke = %.2f, dt = %lums, ETA = %lu", _stroke, dt, _strokeETA);
}

void EaseInModifier::setEaseInSpeed(float easeInSpeed)
{
    _easeInSpeed = easeInSpeed;
    // Update the ETA of the depth and stroke ease-in, should we are in a ramp
    ESP_LOGD("EaseInModifier", "Set Speed = %.2f", _easeInSpeed);
    setDepth(_depth);
    setStroke(_stroke);
}

bool EaseInModifier::applyModification(SafeStrokeParameters_t &parameters)
{
    ESP_LOGD("EaseInModifier", "Apply Modification for Target Position = %.2f, StrokeLength = %.2f, Speed = %.2f, Acceleration = %.2f", parameters.absoluteTargetPosition, parameters.strokeLength, parameters.speed, parameters.acceleration);
    // Update the trapezoidal motion profile with the supplied SafeStrokeParameters_t
    _profile->generateTrapezoidalProfile(parameters.absoluteTargetPosition, parameters.speed, parameters.acceleration);

    // get the ETA of the motion profile
    unsigned long ETA = _profile->getETAAbsolute();
    ESP_LOGD("EaseInModifier", "Stroke End ETA = %lu, Duration %lums", ETA, _profile->getETA());

    if (_depthETA < ETA && _strokeETA < ETA)
    {
        // if the ETA of the depth ease-in or stroke eas-in is shorter than the motion profile ETA
        // the motion profile does not need to be updated
        ESP_LOGD("EaseInModifier", "Already eased in. No modification needed.");
        return false;
    }
    else
    {
        // otherwise the motion profile needs to be updated
        // parameters.targetPosition must be constrained to the depth and stroke limits at ETA
        float targetDepth = _getFutureDepth(ETA);
        float targetStroke = _getFutureStroke(ETA);
        float originalTargetPosition = parameters.absoluteTargetPosition;
        float originalStrokeLength = parameters.strokeLength; // parameters.strokeLength is _stroke in mm and is 0 if the target position is at the end of the stroke
        ESP_LOGD("EaseInModifier", "Original Target Position = %.2f, Original Stroke Length = %.2f", originalTargetPosition, originalStrokeLength);
        ESP_LOGD("EaseInModifier", "Target Depth = %.2f, Target Stroke = %.2f", targetDepth, targetStroke);
        parameters.absoluteTargetPosition = constrain(parameters.absoluteTargetPosition, targetDepth - targetStroke, targetDepth);
        parameters.strokeLength = originalStrokeLength + (parameters.absoluteTargetPosition - originalTargetPosition);
        ESP_LOGD("EaseInModifier", "Constrained Target Position = %.2f, Stroke Length = %.2f", parameters.absoluteTargetPosition, parameters.strokeLength);

        // Adjust acceleration & speed to keep the trapezoidal motion profile shape
        float correction = parameters.strokeLength / originalStrokeLength;
        parameters.acceleration = parameters.acceleration * correction;
        parameters.speed = parameters.speed * correction;
        ESP_LOGD("EaseInModifier", "Adjusted Speed = %.2f, Acceleration = %.2f, Correction factor = %.4f", parameters.speed, parameters.acceleration, correction);

        // Update the trapezoidal motion profile with the updated SafeStrokeParameters_t
        _profile->generateTrapezoidalProfile(parameters.absoluteTargetPosition, parameters.speed, parameters.acceleration);

        return true;
    }
}

float EaseInModifier::_getCurrentDepth()
{
    return _getFutureDepth(millis());
}

float EaseInModifier::_getCurrentStroke()
{
    return _getFutureStroke(millis());
}

float EaseInModifier::_getFutureDepth(unsigned long timeInMs)
{
    if (timeInMs < _depthChangeTime)
    {
        return _previousDepth;
    }
    else if (timeInMs < _depthETA)
    {
        return _previousDepth + signbit(_depth - _previousDepth) * _easeInSpeed * (timeInMs - _depthChangeTime) * 1e-3;
    }
    else
    {
        return _depth;
    }
}

float EaseInModifier::_getFutureStroke(unsigned long timeInMs)
{
    if (timeInMs < _strokeChangeTime)
    {
        return _previousStroke;
    }
    else if (timeInMs < _strokeETA)
    {
        return _previousStroke + signbit(_stroke - _previousStroke) * _easeInSpeed * (timeInMs - _strokeChangeTime) * 1e-3;
    }
    else
    {
        return _stroke;
    }
}

// Path: lib/StrokeEngine/StrokeEngineEaseIn.h
