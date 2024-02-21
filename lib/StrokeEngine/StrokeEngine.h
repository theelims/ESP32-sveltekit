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

#ifndef STROKE_ENGINE_H
#define STROKE_ENGINE_H

#include <StrokeEnginePattern.h>
#include <StrokeEngineMotor.h>
#include <StrokeEngineSafeGuard.h>
#include <StrokeEngineTrapezoidalMotion.h>
#include <StrokeEngineEaseIn.h>
#include <functional>
#include <vector>

#ifndef MOTION_FACTORY_TRAVEL
#define MOTION_FACTORY_TRAVEL 150.0
#endif

#ifndef MOTION_FACTORY_STROKE
#define MOTION_FACTORY_STROKE 80.0
#endif

#ifndef MOTION_FACTORY_RATE
#define MOTION_FACTORY_RATE 30.0
#endif

#ifndef MOTION_FACTORY_SENSATION
#define MOTION_FACTORY_SENSATION 0.0
#endif

#ifndef MOTION_FACTORY_PATTERN
#define MOTION_FACTORY_PATTERN "PoundingTeasing"
#endif

#ifndef MOTION_FACTORY_VIBRATION_AMPLITUDE
#define MOTION_FACTORY_VIBRATION_AMPLITUDE 0.0
#endif

#ifndef MOTION_FACTORY_VIBRATION_FREQUENCY
#define MOTION_FACTORY_VIBRATION_FREQUENCY 30.0
#endif

#ifndef MOTION_FACTORY_EASE_IN_VELOCITY
#define MOTION_FACTORY_EASE_IN_VELOCITY 20.0
#endif

#ifndef MOTION_MAX_RATE
#define MOTION_MAX_RATE 240.0
#endif

#ifndef MOTION_MAX_VELOCITY
#define MOTION_MAX_VELOCITY 500.0
#endif

enum class StrokeParameter
{
  // RATE - Range 0.5 to 600 Strokes / Min
  // Can allow better control typically than just SPEED, as other machines use
  RATE,

  // DEPTH - Range is constrained by motionBounds from MotorInterface
  // Is the point at which the stroke ends
  DEPTH,

  // STROKE - Range is constrained by motionBounds from MotorInterface
  // How far the stroke will retract from DEPTH point
  STROKE,

  // SENSATION - Range is -100 to 100
  // Serves as a generic parameter for usage by patterns to adjust sensation
  SENSATION
};

enum class StrokeLimit
{
  // RATE - Range 0.5 to 600 Strokes / Min
  // Can allow better control typically than just SPEED, as other machines use
  RATE,

  // VELOCITY - Range 0.0 to 2000.0 mm/s
  // Limits the speed a regular motion can reach. Vibrations are not affected.
  VELOCITY,

  // DEPTH - Range is constrained by motionBounds from MotorInterface
  // Is the point at which the stroke ends
  DEPTH,

  // STROKE - Range is constrained by motionBounds from MotorInterface
  // How far the stroke will retract from DEPTH point
  STROKE
};

enum class StrokeCommand
{
  // STOP - Halts any running command and stops the motor immediately.
  STOP,

  // RETRACT - Retracts the motor to the home position with the ease-in speed.
  // This is the default state after the motor is homed.
  RETRACT,

  // DEPTH - Moves the motor to the depth position a.k.a. the end of the
  // stroke with the ease-in speed. Whenever the parameter depth is changed,
  // the motor will move to the new depth.
  DEPTH,

  // STROKE - Moves the motor to the depth-stroke position a.k.a. the
  // start of the stroke with the ease-in speed. Whenever the parameter
  // stroke is changed, the motor will move to the new depth-stroke.
  STROKE,

  // PATTERN - Starts the pattern generator and runs the pattern.
  PATTERN,

  // STROKESTREAM - Starts the stream input and interprets them as
  // trapezoidal moves with 1/3 acceleration, 1/3 constant speed and
  // 1/3 deceleration. The stream input is interpreted as relative
  // positions on the scale [0.0 - 1.0] and mapped to
  // [depth, depth-stroke].
  STROKESTREAM,

  // POSITIONSTREAM - Starts the stream input and interprets them as
  // relative positions on the scale [0.0 - 1.0] and maps them to
  // [depth, depth-stroke].
  POSITIONSTREAM

};

typedef std::function<void(String message)> StrokeEngineNotifyCallback;

/**************************************************************************/
/*!
  @brief  Stroke Engine provides a convenient package for stroking motions
  created by stepper or servo motors. It's internal states are handled by a
  finite state machine. A pattern generator allows to creat a variety of
  motion profiles.
*/
/**************************************************************************/
class StrokeEngine
{
public:
  /**************************************************************************/
  /*!
    @brief  Attach a motor driver to StrokeEngine. The motor driver must
    inherit from MotorInterface. It must be loaded before any motion commands
    are executed.
    @param motor MotorInterface pointer to the motor driver instance
  */
  /**************************************************************************/
  void attachMotor(MotorInterface *motor);

  /**************************************************************************/
  /*!
  @brief  Returns an instance of the attached motor driver
  @return MotorInterface pointer to the motor driver instance
  */
  /**************************************************************************/
  MotorInterface *getMotor() { return _motor; }

  /**************************************************************************/
  /*!
    @brief  Starts the StrokeEngine. This will start the motor and the
    pattern generator. The motor will be homed if it is not already. To make
    the motor follow changes on the depth and stroke parameters, the runCommand
    method with StrokeCommand::DEPTH or StrokeCommand::STROKE must be called
    again after changing depth or stroke.
    @param  command The StrokeCommand to execute
    @return TRUE on success, FALSE if command is not possible
  */
  /**************************************************************************/
  bool runCommand(StrokeCommand command);
  StrokeCommand getCommand();

  /**************************************************************************/
  /*!
    @brief Sets an internal Pattern Parameter. Takes effect on next stroke unless
    applyChangesNow() is called.
    @param value See StrokeParameter documentation for constraints placed on value
    @return the actually used value after sanitizing / cropping the input
  */
  /**************************************************************************/
  float setParameter(StrokeParameter parameter, float value);
  float getParameter(StrokeParameter parameter);

  /**************************************************************************/
  /*!
    @brief Sets an internal limit Parameter. Takes effect on next stroke unless
    applyChangesNow() is called.
    @param value See StrokeLimit documentation for constraints placed on value
    @return the actually used value after sanitizing / cropping the input
  */
  /**************************************************************************/
  float setLimit(StrokeLimit limit, float value);
  float getLimit(StrokeLimit limit);

  /**************************************************************************/
  /*!
    @brief Sets the ease-in velocity for StrokeEngine. This is the speed at
    which the motor will move to the depth or stroke position after these have
    been changed. Takes effect on next stroke unless applyChangesNow() is called.
    @param value The ease-in velocity in mm/s
    @return the actually used value after sanitizing / cropping the input
  */
  /**************************************************************************/
  float setEaseInVelocity(float value);
  float getEaseInVelocity();

  /**************************************************************************/
  /*! @brief Applies changes from setParameter() and setLimit() immediately */
  /**************************************************************************/
  void applyChangesNow();

  /**************************************************************************/
  /*!
    @brief  Choose a pattern for the StrokeEngine. Settings take effect with
    next stroke, or with applyNow set to true immediately.
    @param patternIndex  Index of a pattern
    @param applyNow Set to true if changes should take effect immediately
    @return TRUE on success, FALSE, if patternIndex is invalid. Previous
                  pattern will be retained.
  */
  /**************************************************************************/
  bool setPattern(int patternIndex, bool applyNow = true);

  /**************************************************************************/
  /*!
    @brief  Choose a pattern for the StrokeEngine. Settings take effect with
    next stroke, or with applyNow set to true immediately.
    @param patternName  Name of a pattern
    @param applyNow Set to true if changes should take effect immediately
    @return TRUE on success, FALSE, if patternIndex is invalid. Previous
                  pattern will be retained.
  */
  /**************************************************************************/
  bool setPattern(String patternName, bool applyNow = true);

  /**************************************************************************/
  /*!
    @brief  Get the current pattern index for the StrokeEngine.
    @return Index of a pattern
  */
  /**************************************************************************/
  int getCurrentPattern();

  /**************************************************************************/
  /*!
    @brief  Get the current pattern name for the StrokeEngine.
    @return Name of a pattern
  */
  /**************************************************************************/
  String getCurrentPatternName();

  /**************************************************************************/
  /*!
    @brief  Allows fetching Pattern names for UIs
    @param index index of a pattern.
    @return String holding a pattern name with a certain index. If index is
                  out of range it returns "Invalid"
  */
  /**************************************************************************/
  String getPatternName(int index);

  /**************************************************************************/
  /*!
    @brief  Allows fetching Pattern names for UIs without going out of bounds
    @return The number of patterns available.
  */
  /**************************************************************************/
  unsigned int getNumberOfPattern() { return patternTableSize; }

  /**************************************************************************/
  /*!
    @brief  Registers a callback that will be called when the StrokeEngine
            changes something internally and the UI should be updated.
    @param callback Function pointer to a function that takes a String as
                    argument.
  */
  /**************************************************************************/
  void onNotify(StrokeEngineNotifyCallback callback);

  /**************************************************************************/
  /*!
    @brief  Returns wether StrokeEngine is running a pattern or is streaming.
    @return TRUE if the pattern or streaming are active, FALSE if it is not.
  */
  /**************************************************************************/
  bool isActive() { return _active; }

protected:
  bool _active = false;
  MotorInterface *_motor;
  StrokeEngineSafeGuard _safeGuard; // all setting & makeSafe calls must be made within the scope of a taken _parameterMutex
  StrokeCommand _command = StrokeCommand::STOP;
  TrapezoidalMotion _trapezoidalProfile;
  EaseInModifier _easeIn = EaseInModifier(&_trapezoidalProfile, &_safeGuard);

  int _patternIndex = 0;
  int _index = 0;

  float _sensation;

  bool _applyUpdate = false;

  void _startPattern();
  void _stopMotion();
  void _updateFixedPosition();

  SemaphoreHandle_t _parameterMutex = xSemaphoreCreateMutex();
  void _sendParametersToPattern(int patternIndex);

  static void _strokingImpl(void *_this) { static_cast<StrokeEngine *>(_this)->_stroking(); }
  void _stroking();
  TaskHandle_t _taskStrokingHandle = NULL;

  std::vector<StrokeEngineNotifyCallback> _onNotifyCallbacks;
  void _notify(String message);
};

#endif // STROKE_ENGINE_H