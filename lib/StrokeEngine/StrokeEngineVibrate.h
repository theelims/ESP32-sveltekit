/**
 *   Vibration Modifier of StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 - 2024 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef VIBRATE_H
#define VIBRATE_H

#include <Arduino.h>
#include <StrokeEngineTrapezoidalMotion.h>
#include <StrokeEngine.h>
#include <StrokeEngineMotor.h>
#include <StrokeEngineSafeGuard.h>

/**
 * @brief VibrationModifier
 * Modifies a move with a vibration frequency and amplitude. If not manually overridden, the vibration parameters supplied by the parameters are used.
 * In manual override mode, the vibration parameters are set by the user are used. Setting the amplitude to 0 disables the vibration. This modifier
 * talks directly to the motor and emulates the motors behaviour upstream. If vibrations are disabled, the parameters are passed through unchanged.
 * If vibrations are enabled, the TrapezoidalMotion profile is used for the main motion and each vibration motion is passed to the motor.
 *
 * motionCompleted() returns only true if the main motion is completed and must be called frequently to pass on the vibration motion to the motor.
 *
 * In state StrokeCommand::POSITIONSTREAM vibrations are generally disabled.
 */

class VibrationModifier
{
public:
    VibrationModifier(TrapezoidalMotion *profile, MotorInterface *motor, StrokeCommand *command);
    ~VibrationModifier(){};

    void setVibrationFrequency(float vibrationFrequency);
    void setVibrationAmplitude(float vibrationAmplitude);
    void manualOverride(bool override);

    void applyVibrationModification(SafeStrokeParameters_t &parameters);

    bool motionCompleted();

private:
    TrapezoidalMotion *_profile;
    MotorInterface *_motor;
    StrokeCommand *_command;
    float _vibrationFrequency;
    float _vibrationAmplitude;
    bool _manualOverride;
};

#endif // VIBRATE_H
