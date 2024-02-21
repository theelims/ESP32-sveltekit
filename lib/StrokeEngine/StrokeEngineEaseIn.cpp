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
}

void EaseInModifier::setDepth(float depth)
{
    _previousDepth = _depth;
    _depth = depth;
    _depthChangeTime = millis();
    _depthETA = _depthChangeTime + (unsigned int)(abs(_depth - _previousDepth) / _easeInSpeed * 1000.0);
}

void EaseInModifier::setStroke(float stroke)
{
    _previousStroke = _stroke;
    _stroke = stroke;
    _strokeChangeTime = millis();
    _strokeETA = _strokeChangeTime + (unsigned int)((_stroke - _previousStroke) / _easeInSpeed * 1000.0);
}

void EaseInModifier::setEaseInSpeed(float easeInSpeed)
{
    _easeInSpeed = easeInSpeed;
    // Update the ETA of the depth and stroke ease-in, should we are in a ramp
    setDepth(_depth);
    setStroke(_stroke);
}

bool EaseInModifier::applyModification(SafeStrokeParameters_t &parameters)
{
    // Update the trapezoidal motion profile with the supplied SafeStrokeParameters_t
    _profile->generateTrapezoidalProfile(parameters.absoluteTargetPosition, parameters.speed, parameters.acceleration);

    // get the ETA of the motion profile
    unsigned long ETA = _profile->getETAAbsolute();

    if (_depthETA < ETA && _strokeETA < ETA)
    {
        // if the ETA of the depth ease-in or stroke eas-in is shorter than the motion profile ETA
        // the motion profile does not need to be updated
        return false;
    }
    else
    {
        // otherwise the motion profile needs to be updated
        // parameters.targetPosition must be constrained to the depth and stroke limits at ETA
        float targetDepth = _getFutureDepth(ETA);
        float targetStroke = _getFutureStroke(ETA);
        float originalTargetPosition = parameters.absoluteTargetPosition;
        float originalStrokeLength = parameters.strokeLength;
        parameters.absoluteTargetPosition = constrain(parameters.absoluteTargetPosition, targetDepth - targetStroke, targetDepth);
        parameters.strokeLength = originalStrokeLength + (parameters.absoluteTargetPosition - originalTargetPosition);

        // Adjust acceleration & speed to keep the trapezoidal motion profile shape
        float correction = parameters.strokeLength / originalStrokeLength;
        parameters.acceleration = parameters.acceleration * correction;
        parameters.speed = parameters.speed * correction;

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
        return _previousDepth + signbit(_depth - _previousDepth) * _easeInSpeed * (timeInMs - _depthChangeTime);
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
        return _previousStroke + signbit(_stroke - _previousStroke) * _easeInSpeed * (timeInMs - _strokeChangeTime);
    }
    else
    {
        return _stroke;
    }
}

// Path: lib/StrokeEngine/StrokeEngineEaseIn.h
