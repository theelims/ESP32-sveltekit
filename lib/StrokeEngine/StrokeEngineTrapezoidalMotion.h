/**
 *   Trapezoidal Profile Generator of StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 - 2024 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TRAPEZOIDAL_MOTION_H
#define TRAPEZOIDAL_MOTION_H

#include <math.h>
#include <freertos/semphr.h>

/**************************************************************************/
/*!
  @brief  Struct defining a speed and position tuple
*/
/**************************************************************************/
struct speedAndPosition
{
    float speed;        /*> Speed in [mm/s] */
    float position;     /*> Position in [mm] */
    float acceleration; /*> Acceleration in [mm/s²] */
};

/**************************************************************************/
/*!
  @brief  Struct defining a point of a trapezoidal motion profile
*/
/**************************************************************************/
struct trapezoidalRampPoint
{
    float time;     /*> Time in [s] */
    float position; /*> Position in [mm] */
    float speed;    /*> Speed in [mm/s] */
};

/**************************************************************************/
/*!
  @brief  Calculates a trapezoidal motion profile which can be followed by
  calling the getSpeedAndPosition function. The profile is generated
  based on the given position, speed and acceleration and takes the current
  motion into account. If the previous profile is not completed, the new
  profile will be generated based on the current position and speed.
  The profile is generated in a way that the motor will never exceed the
  given speed and acceleration.
  In this case a trapezoidal motion always consists of these phases:
  Now --[0]--> Deceleration --[1]--> Acceleration --[2]--> Coasting
  --[3]--> Deceleration to zero --[4]--> stand still / motion completed
  Depending on the conditions certain phases have the time=0 and are
  effectively skipped.
*/
/**************************************************************************/
class TrapezoidalMotion
{
public:
    /**
     * @brief Default constructor.
     */
    TrapezoidalMotion();

    /**
     * @brief Destructor.
     */
    ~TrapezoidalMotion(){};

    /**
     * @brief Resets the profile to a new position.
     *
     * This method stops the current motion and sets everything to stand still.
     *
     * @param position The position in [mm] to reset the profile to.
     */
    void resetProfileTo(float position);

    /**
     * @brief Generates a trapezoidal motion profile.
     *
     * This method generates a trapezoidal motion profile based on the given position,
     * speed, and acceleration. The profile is generated in a way that the motor will
     * never exceed the given speed and acceleration. The profile is generated based on
     * the current position and speed.
     *
     * @param position The target position in [mm].
     * @param speed The target speed in [mm/s].
     * @param acceleration The target acceleration in [mm/s²].
     */
    void generateTrapezoidalProfile(float position, float speed, float acceleration);

    /**
     * @brief Gets the speed and position at an absolute time.
     *
     * This method returns the speed and position of the motor at the specified absolute time.
     *
     * @param absoluteTimeInMs The absolute time in milliseconds.
     * @return The speed and position at the specified time.
     */
    speedAndPosition getSpeedAndPositionAbsolute(unsigned int absoluteTimeInMs);

    /**
     * @brief Gets the speed and position at a time since the start of the motion.
     *
     * This method returns the speed and position of the motor at the specified time since
     * the start of the motion.
     *
     * @param timeSinceStartInMs The time since the start of the motion in milliseconds.
     * @return The speed and position at the specified time.
     */
    speedAndPosition getSpeedAndPosition(unsigned int timeSinceStartInMs);

    /**
     * @brief Gets the acceleration of the motion.
     *
     * This method returns the acceleration of the motion.
     *
     * @return The acceleration in [mm/s²].
     */
    float getAcceleration();

    /**
     * @brief Gets a profile point at the specified index.
     *
     * This method returns a profile point at the specified index.
     *
     * @param index The index of the profile point.
     * @return The profile point at the specified index.
     */
    trapezoidalRampPoint getProfilePoint(unsigned int index);

    /**
     * @brief Gets the estimated time of arrival (ETA) for the motion.
     *
     * This method returns the estimated time of arrival (ETA) for the motion.
     *
     * @return The estimated time of arrival (ETA) in milliseconds.
     */
    unsigned int getETA();

    /**
     * @brief Gets the absolute estimated time of arrival (ETA) for the motion.
     *
     * This method returns the absolute estimated time of arrival (ETA) for the motion.
     *
     * @return The absolute estimated time of arrival (ETA) in milliseconds.
     */
    unsigned int getETAAbsolute();

    /**
     * @brief Checks if the motion is completed.
     *
     * This method checks if the motion is completed.
     *
     * @return True if the motion is completed, false otherwise.
     */
    bool motionCompleted();

protected:
    bool _motionCompleted = true;
    unsigned int _startOfProfileInMs = 0;
    float _acceleration = 0.0;
    int _profilePhaseDebugMessages = 0;
    trapezoidalRampPoint _trapezoidalProfile[5];
    SemaphoreHandle_t _parameterMutex = xSemaphoreCreateMutex();
};

#endif // TRAPEZOIDAL_MOTION_H