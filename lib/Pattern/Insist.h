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

#ifndef INSIST_H
#define INSIST_H

#include <Pattern.h>

/**************************************************************************/
/*!
  @brief  Sensation reduces the effective stroke length while keeping the
  stroke speed constant to the full stroke. This creates interesting
  vibrational pattern at higher sensation values. With positive sensation the
  strokes will wander towards the front, with negative values towards the back.
*/
/**************************************************************************/
class Insist : public Pattern
{
public:
    Insist(const char *str) : Pattern(str) {}

    void setSensation(float sensation)
    {
        _sensation = sensation;

        // make invert sensation and make into a fraction of the stroke distance
        _strokeFraction = (100 - abs(sensation)) / 100.0f;

        _strokeInFront = (sensation > 0) ? true : false;

        _updateStrokeTiming();
    }

    void setTimeOfStroke(float speed = 0)
    {
        // In & Out have same time, so we need to divide by 2
        _timeOfStroke = 0.5 * speed;
        _updateStrokeTiming();
    }

    void setStroke(int stroke)
    {
        _stroke = stroke;
        _updateStrokeTiming();
    }

    motionParameters_t nextTarget(unsigned int index, bool retract = false)
    {

        // acceleration & speed to meet the profile
        _nextMove.acceleration = _acceleration;
        _nextMove.speed = _speed;

        if (_strokeInFront)
        {
            // odd stroke is moving out
            if (index % 2)
            {
                _nextMove.stroke = _stroke - _realStroke;

                // even stroke is moving in
            }
            else
            {
                _nextMove.stroke = _stroke;
            }
        }
        else
        {
            // odd stroke is moving out
            if (index % 2)
            {
                _nextMove.stroke = 0;

                // even stroke is moving in
            }
            else
            {
                _nextMove.stroke = _realStroke;
            }
        }

        _index = index;

        return _nextMove;
    }

protected:
    float _speed = 0.0;
    float _acceleration = 0.0;
    float _realStroke = 0.0;
    float _strokeFraction = 1.0;
    bool _strokeInFront = false;
    void _updateStrokeTiming()
    {
        // maximum speed of the longest trapezoidal motion (full stroke)
        _speed = 1.5 * _stroke / _timeOfStroke;

        // Acceleration to hold 1/3 profile with fractional strokes
        _acceleration = 3.0 * _speed / (_timeOfStroke * _strokeFraction);

        // Calculate fractional stroke length
        _realStroke = _stroke * _strokeFraction;
    }
};

#endif // INSIST_H