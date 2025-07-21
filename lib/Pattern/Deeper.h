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

#ifndef DEEPER_H
#define DEEPER_H

#include <Pattern.h>

/**************************************************************************/
/*!
  @brief  The insertion depth ramps up gradually with each stroke until it
  reaches its maximum. It then resets and restarts. Sensations controls how
  many strokes there are in a ramp.
*/
/**************************************************************************/
class Deeper : public Pattern
{
public:
    Deeper(const char *str) : Pattern(str) {}

    void setTimeOfStroke(float speed = 0)
    {
        // In & Out have same time, so we need to divide by 2
        _timeOfStroke = 0.5 * speed;
    }

    void setSensation(float sensation)
    {
        _sensation = sensation;

        // maps sensation to useful values [2,22] with 12 beeing neutral
        if (sensation < 0)
        {
            _countStrokesForRamp = map(sensation, -100, 0, 2, 11);
        }
        else
        {
            _countStrokesForRamp = map(sensation, 0, 100, 11, 32);
        }
    }

    motionParameters_t nextTarget(unsigned int index, bool retract = false)
    {
        // How many steps is each stroke advancing
        float slope = _stroke / float(_countStrokesForRamp);

        // The pattern recycles so we use modulo to get a cycling index.
        // Factor 2 because index increments with each full stroke twice
        // add 1 because modulo = 0 is index = 1
        int cycleIndex = (index / 2) % _countStrokesForRamp + 1;

        // This might be not smooth, as the insertion depth may jump when
        // sensation is adjusted.

        // Amplitude is slope * cycleIndex
        float amplitude = slope * float(cycleIndex);

        // maximum speed of the trapezoidal motion
        _nextMove.speed = 1.5 * amplitude / _timeOfStroke;

        // acceleration to meet the profile
        _nextMove.acceleration = 3.0 * _nextMove.speed / _timeOfStroke;

        // odd stroke is moving out
        if (index % 2)
        {
            _nextMove.stroke = 0;

            // even stroke is moving in
        }
        else
        {
            _nextMove.stroke = amplitude;
        }

        _index = index;
        return _nextMove;
    }

protected:
    int _countStrokesForRamp = 2;
};

#endif // DEEPER_H