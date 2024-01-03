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

#pragma once

#include <pattern.h>
#include <motor/motor.h>

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

#ifndef MOTION_FACTORY_MAX_RATE
#define MOTION_FACTORY_MAX_RATE 240.0
#endif

enum class StrokeParameter
{
  // RATE - Range 0.5 to 6000 Strokes / Min
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
  // RATE - Range 0.5 to 6000 Strokes / Min
  // Can allow better control typically than just SPEED, as other machines use
  RATE,

  // DEPTH - Range is constrained by motionBounds from MotorInterface
  // Is the point at which the stroke ends
  DEPTH,

  // STROKE - Range is constrained by motionBounds from MotorInterface
  // How far the stroke will retract from DEPTH point
  STROKE
};

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
  void attachMotor(MotorInterface *motor);
  MotorInterface *getMotor() { return _motor; }

  /*!
    @brief Sets an internal Pattern Parameter. Takes effect on next stroke unless applyNow issued
    @param value See StrokeParameter documentation for constraints placed on value
    @param applyNow Set to true if changes should take effect immediately
    @return the actually used value after sanitizing / cropping the input
  */
  float setParameter(StrokeParameter parameter, float value, bool applyNow = true);
  float getParameter(StrokeParameter parameter);

  /*!
    @brief Sets an internal limit Parameter. Takes effect on next stroke unless applyNow issued
    @param value See StrokeLimit documentation for constraints placed on value
    @param applyNow Set to true if changes should take effect immediately
    @return the actually used value after sanitizing / cropping the input
  */
  float setLimit(StrokeLimit limit, float value, bool applyNow = true);
  float getLimit(StrokeLimit limit);

  /*!
    @brief  Starts the internal motion loop which feeds Movements generated by the Pattern to the attached Motor
    @return if True, indicates Pattern has started successfully
  */
  bool startPattern();

  /*! @brief Stops Pattern execution, and instructs the motor to immediately stop motion */
  void stopMotion();

  /**************************************************************************/
  /*!
    @brief  Choose a pattern for the StrokeEngine. Settings take effect with
    next stroke, or after calling applyNewSettingsNow().
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
    next stroke, or after calling applyNewSettingsNow().
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

  /*!
    @brief  Allows fetching Pattern names for UIs without going out of bounds
    @return The number of patterns available.
  */
  unsigned int getNumberOfPattern() { return patternTableSize; }

  bool isActive() { return _active; }

protected:
  bool _active = false;
  MotorInterface *_motor;

  int _patternIndex = 0;
  int _index = 0;

  float _depth;
  float _depthLimit = MOTION_FACTORY_TRAVEL;
  float _stroke;
  float _strokeLimit = MOTION_FACTORY_TRAVEL;
  float _timeOfStroke;
  float _timeOfStrokeLimit = 60.0 / MOTION_FACTORY_RATE;
  float _sensation;

  bool _applyUpdate = false;

  SemaphoreHandle_t _parameterMutex = xSemaphoreCreateMutex();
  void _sendParameters(int patternIndex);

  static void _strokingImpl(void *_this) { static_cast<StrokeEngine *>(_this)->_stroking(); }
  void _stroking();
  TaskHandle_t _taskStrokingHandle = NULL;
};
