/**
 *   Patterns of the StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2025 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef RANDOMDEPTH_H
#define RANDOMDEPTH_H

#include <Pattern.h>

/**************************************************************************/
/*!
  @brief  Like Teasing or Pounding, but every every stroke has a radom
  depth between 10% and 100% of the stroke length. The sensation value
  can change the speed ratio between in and out. Sensation > 0 make the in
  move faster (up to 2x) giving a hard pounding sensation. Values < 0 make
  the out move going faster. This gives a more pleasing sensation. The
  speed for the stroke remains the same as for a full stroke.
*/
/**************************************************************************/
class RandomDepth : public Pattern
{
public:
    RandomDepth(const char *str) : Pattern(str) {}
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
        // odd stroke is moving out
        if (index % 2)
        {
            // maximum speed of the trapezoidal motion for the full stroke length
            _nextMove.speed = 1.5 * _stroke / _timeOfOutStroke;

            // acceleration to meet the profile for the shorter random stroke
            _nextMove.acceleration = 3.0 * _nextMove.speed / (_timeOfOutStroke * _randomFactor);
            _nextMove.stroke = 0;
        }
        // even stroke is moving in
        else
        {
            // get new random value
            _randomFactor = random(10, 100) / 100.0;

            // maximum speed of the trapezoidal motion for the full stroke length
            _nextMove.speed = 1.5 * _stroke / _timeOfInStroke;

            // acceleration to meet the profile for the shorter random stroke
            _nextMove.acceleration = 3.0 * _nextMove.speed / (_timeOfInStroke * _randomFactor);
            _nextMove.stroke = _stroke * _randomFactor;
        }
        _index = index;
        return _nextMove;
    }

protected:
    float _timeOfFastStroke = 1.0;
    float _timeOfInStroke = 1.0;
    float _timeOfOutStroke = 1.0;
    float _randomFactor = 1.0;
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
        }
        // negative sensation, out is faster
        else
        {
            _timeOfOutStroke = _timeOfFastStroke;
            _timeOfInStroke = _timeOfStroke - _timeOfFastStroke;
        }
    }
};

#endif // RANDOMDEPTH_H