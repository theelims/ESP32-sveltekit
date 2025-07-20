/**
 *   Patterns of the StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2024 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef PATTERN_H
#define PATTERN_H

#include <Arduino.h>
#include <math.h>
#include <PatternMath.h>

#ifndef STRING_LEN
#define STRING_LEN 64 // Bytes used to initialize char array. No path, topic, name, etc. should exceed this value
#endif

/**************************************************************************/
/*!
  @brief  struct to return all parameters FastAccelStepper needs to calculate
  the trapezoidal profile.
*/
/**************************************************************************/
typedef struct
{
    float stroke;       //!< Absolute and properly constrained target position of a move in [mm}
    float speed;        //!< Speed of a move in [mm/s]
    float acceleration; //!< Acceleration to get to speed or halt in [mm/s^2]
    bool skip;          //!< no valid stroke, skip this set an query for the next --> allows pauses between strokes
} motionParameters_t;

/**************************************************************************/
/*!
  @class Pattern
  @brief  Base class to derive your pattern from. Offers a unified set of
          functions to store all relevant parameters. These function can be
          overriden if necessary. Pattern should be self-contained and not
          rely on any stepper/servo related properties. Internal book keeping
          is done in steps. The translation from real word units to steps is
          provided by the StrokeEngine. Also the sanity check whether motion
          parameters are physically possible are done by the StrokeEngine.
          Impossible motion commands are clipped, cropped or adjusted while
          still having a smooth appearance.
*/
/**************************************************************************/
class Pattern
{

public:
    //! Constructor
    /*!
      @param str String containing the name of a pattern
    */
    Pattern(const char *str) { strcpy(_name, str); }

    //! Set the time a normal stroke should take to complete
    /*!
      @param speed time of a full stroke in [sec]
    */
    virtual void setTimeOfStroke(float speed) { _timeOfStroke = speed; }

    //! Set the maximum stroke a pattern may have
    /*!
      @param stroke stroke distance in mm
    */
    virtual void setStroke(float stroke) { _stroke = stroke; }

    //! Sensation is an additional parameter a pattern can take to alter its behaviour
    /*!
      @param sensation Arbitrary value from -100 to 100, with 0 beeing neutral
    */
    virtual void setSensation(float sensation) { _sensation = sensation; }

    //! Retrieves the name of a pattern
    /*!
      @return c_string containing the name of a pattern
    */
    char *getName() { return _name; }

    //! Calculate the position of the next stroke based on the various parameters
    /*!
      @param index index of a stroke. Increments with every new stroke.
      @param retract this is a hint StrokeEngine gives
      @return Set of motion parameters like speed, acceleration & position
    */
    virtual motionParameters_t nextTarget(unsigned int index, bool retract = false)
    {
        _index = index;
        return _nextMove;
    }

protected:
    float _stroke;
    float _timeOfStroke;
    float _sensation = 0.0;
    const float _infinite = 1.0e30; // an approximation for infinite should maximum machine speed and acceleration are needed.
    int _index = -1;
    char _name[STRING_LEN];
    motionParameters_t _nextMove = {0.0, 0.0, 0.0, false};
    int _startDelayMillis = 0;
    int _delayInMillis = 0;

    /*!
      @brief Start a delay timer which can be polled by calling _isStillDelayed().
      Uses internally the millis()-function.
    */
    void _startDelay()
    {
        _startDelayMillis = millis();
    }

    /*!
      @brief Update a delay timer which can be polled by calling _isStillDelayed().
      Uses internally the millis()-function.
      @param delayInMillis delay in milliseconds
    */
    void _updateDelay(int delayInMillis)
    {
        _delayInMillis = delayInMillis;
    }

    /*!
      @brief Poll the state of a internal timer to create pauses between strokes.
      Uses internally the millis()-function.
      @return True, if the timer is running, false if it is expired.
    */
    bool _isStillDelayed()
    {
        return (millis() > (_startDelayMillis + _delayInMillis)) ? false : true;
    }
};

/**************************************************************************/
/*!
  TODO: New Pattern Ideas
    - KnottyJoy: A series of rapid 50% depth strokes followed by a regular
                speed 100% stroke and a pause. Sensation controls the number
                of rapid strokes and the hold time.
*/
/**************************************************************************/

#endif // PATTERN_H