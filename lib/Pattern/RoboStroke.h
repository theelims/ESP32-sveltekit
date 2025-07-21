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

#ifndef ROBOSTROKE_H
#define ROBOSTROKE_H

#include <Pattern.h>

/**************************************************************************/
/*!
  @brief  Robot Stroke Pattern. Sensation controls the acceleration of the
  stroke. Positive value increase acceleration until it is a constant speed
  motion (feels robotic). Neutral is equal to simple stroke (1/3, 1/3, 1/3).
  Negative reduces acceleration into a triangle profile.
*/
/**************************************************************************/
class RoboStroke : public Pattern
{
public:
    RoboStroke(const char *str) : Pattern(str) {}

    void setTimeOfStroke(float speed = 0)
    {
        // In & Out have same time, so we need to divide by 2
        _timeOfStroke = 0.5 * speed;
    }

    void setSensation(float sensation = 0)
    {
        _sensation = sensation;
        // scale sensation into the range [0.05, 0.5] where 0 = 1/3
        if (sensation >= 0)
        {
            _x = fscale(0.0, 100.0, 1.0 / 3.0, 0.5, sensation, 0.0);
        }
        else
        {
            _x = fscale(0.0, 100.0, 1.0 / 3.0, 0.05, -sensation, 0.0);
        }
    }

    motionParameters_t nextTarget(unsigned int index, bool retract = false)
    {
        // maximum speed of the trapezoidal motion
        _nextMove.speed = _stroke / ((1 - _x) * _timeOfStroke);

        // acceleration to meet the profile
        _nextMove.acceleration = _nextMove.speed / (_x * _timeOfStroke);

        // odd stroke is moving out
        if (index % 2)
        {
            _nextMove.stroke = 0;

            // even stroke is moving in
        }
        else
        {
            _nextMove.stroke = _stroke;
        }

        _index = index;
        return _nextMove;
    }

protected:
    float _x = 1.0 / 3.0;
};

#endif // ROBOSTROKE_H