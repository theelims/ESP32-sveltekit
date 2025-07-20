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

#ifndef HALFNHALF_H
#define HALFNHALF_H

#include <Pattern.h>

/**************************************************************************/
/*!
  @brief  Like Teasing or Pounding, but every second stroke is only half the
  depth. The sensation value can change the speed ratio between in and out.
  Sensation > 0 make the in move faster (up to 2x) giving a hard pounding
  sensation. Values < 0 make the out move going faster. This gives a more
  pleasing sensation. The time for the overall stroke remains the same for
  all strokes, even half ones.
*/
/**************************************************************************/
class HalfnHalf : public Pattern
{
public:
    HalfnHalf(const char *str) : Pattern(str) {}
    void setSensation(float sensation)
    {
        _sensation = sensation;
        _updateStrokeTiming();
    }
    void setTimeOfStroke(float speed = 0)
    {
        _timeOfStroke = speed;
        _updateStrokeTiming();
    }
    motionParameters_t nextTarget(unsigned int index, bool retract = false)
    {
        // check if this is the very first
        if (index == 0)
        {
            // pattern started for the very fist time, so we start gentle with a half move
            _half = true;
        }

        // set-up the stroke length
        float stroke = _stroke;
        if (_half == true)
        {
            // half the stroke length
            stroke = _stroke / 2;
        }

        // odd stroke is moving out
        if (index % 2)
        {
            // maximum speed of the trapezoidal motion
            _nextMove.speed = 1.5 * stroke / _timeOfOutStroke;

            // acceleration to meet the profile
            _nextMove.acceleration = 3.0 * _nextMove.speed / _timeOfOutStroke;
            _nextMove.stroke = 0;
            // every second move is half
            _half = !_half;
            // even stroke is moving in
        }
        else
        {
            // maximum speed of the trapezoidal motion
            _nextMove.speed = 1.5 * stroke / _timeOfInStroke;

            // acceleration to meet the profile
            _nextMove.acceleration = 3.0 * _nextMove.speed / _timeOfInStroke;
            _nextMove.stroke = stroke;
        }
        _index = index;
        return _nextMove;
    }

protected:
    float _timeOfFastStroke = 1.0;
    float _timeOfInStroke = 1.0;
    float _timeOfOutStroke = 1.0;
    bool _half = true;
    void _updateStrokeTiming()
    {
        // calculate the time it takes to complete the faster stroke
        // Division by 2 because reference is a half stroke
        _timeOfFastStroke = (0.5 * _timeOfStroke) / fscale(0.0, 100.0, 1.0, 3.0, abs(_sensation), -2.0);
        // positive sensation, in is faster
        if (_sensation > 0.0)
        {
            _timeOfInStroke = _timeOfFastStroke;
            _timeOfOutStroke = _timeOfStroke - _timeOfFastStroke;
            // negative sensation, out is faster
        }
        else
        {
            _timeOfOutStroke = _timeOfFastStroke;
            _timeOfInStroke = _timeOfStroke - _timeOfFastStroke;
        }
    }
};

#endif // HALFNHALF_H