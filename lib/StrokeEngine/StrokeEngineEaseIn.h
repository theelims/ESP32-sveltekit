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

#ifndef EASE_IN_H
#define EASE_IN_H

#include <Arduino.h>
#include <StrokeEngineTrapezoidalMotion.h>
#include <StrokeEngineSafeGuard.h>

/**
 * @brief EaseInModifier
 * Stretches or shortens a move so that changes in stroke and depth are only propagated with the easeInSpeed.
 * Acceleration and deceleration are adjusted to keep the trapezoidal motion profiles shape. Speed remains
 * unchanged to obey the speed limit. To calculate the motion duration the trapezoidal motion profile is updated
 * with the supplied SafeStrokeParameters_t.
 */
class EaseInModifier
{
public:
    EaseInModifier(TrapezoidalMotion *profile, StrokeEngineSafeGuard *safeGuard);
    ~EaseInModifier(){};

    void begin();

    void setDepth(float depth);
    void setStroke(float stroke);
    void setEaseInSpeed(float easeInSpeed);

    bool applyModification(SafeStrokeParameters_t &parameters);

private:
    TrapezoidalMotion *_profile;
    StrokeEngineSafeGuard *_safeGuard;

    float _easeInSpeed;

    float _depth;
    float _previousDepth;
    unsigned long _depthChangeTime;
    unsigned long _depthETA;

    float _stroke;
    float _previousStroke;
    unsigned long _strokeChangeTime;
    unsigned long _strokeETA;

    float _getCurrentDepth();
    float _getCurrentStroke();

    float _getFutureDepth(unsigned long timeInMs);
    float _getFutureStroke(unsigned long timeInMs);
};

#endif // EASE_IN_H
