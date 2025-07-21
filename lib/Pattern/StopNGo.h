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

#ifndef STOPNGO_H
#define STOPNGO_H

#include <Pattern.h>

/**************************************************************************/
/*!
  @brief  Pauses between a series of strokes.
  The number of strokes ramps from 1 stroke to 5 strokes and back. Sensation
  changes the length of the pauses between stroke series.
*/
/**************************************************************************/
class StopNGo : public Pattern
{
public:
    StopNGo(const char *str) : Pattern(str) {}

    void setTimeOfStroke(float speed = 0)
    {
        // In & Out have same time, so we need to divide by 2
        _timeOfStroke = 0.5 * speed;
    }

    void setSensation(float sensation)
    {
        _sensation = sensation;

        // maps sensation to a delay from 100ms to 10 sec
        _updateDelay(map(sensation, -100, 100, 100, 10000));
    }

    motionParameters_t nextTarget(unsigned int index, bool retract = false)
    {
        // maximum speed of the trapezoidal motion
        _nextMove.speed = 1.5 * _stroke / _timeOfStroke;

        // acceleration to meet the profile
        _nextMove.acceleration = 3.0 * _nextMove.speed / _timeOfStroke;

        // adds a delay between each stroke
        if (_isStillDelayed() == false)
        {

            // odd stroke is moving out
            if (index % 2)
            {
                _nextMove.stroke = 0;

                if (_strokeIndex >= _strokeSeriesIndex)
                {
                    // Reset stroke index to 1
                    _strokeIndex = 0;

                    // change count direction once we reached the maximum number of strokes
                    if (_strokeSeriesIndex >= _numberOfStrokes)
                    {
                        _countStrokesUp = false;
                    }

                    // change count direction once we reached one stroke counting down
                    if (_strokeSeriesIndex <= 1)
                    {
                        _countStrokesUp = true;
                    }

                    // increment or decrement strokes counter
                    if (_countStrokesUp == true)
                    {
                        _strokeSeriesIndex++;
                    }
                    else
                    {
                        _strokeSeriesIndex--;
                    }

                    // start delay after having moved out
                    _startDelay();
                }

                // even stroke is moving in
            }
            else
            {
                _nextMove.stroke = _stroke;
                // Increment stroke index by one
                _strokeIndex++;
            }
            _nextMove.skip = false;
        }
        else
        {
            _nextMove.skip = true;
        }

        _index = index;

        return _nextMove;
    }

protected:
    int _numberOfStrokes = 5;
    int _strokeSeriesIndex = 1;
    int _strokeIndex = 0;
    bool _countStrokesUp = true;
};

#endif // STOPNGO_H