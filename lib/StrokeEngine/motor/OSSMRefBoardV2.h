/**
 *   Generic Stepper Driver of StrokeEngine for the OSSM ref board V2
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#pragma once

#include "genericStepper.h"

/**
 * @brief Structure representing the properties of the OSSMRefBoardV2.
 *
 * This structure contains the configuration parameters for the OSSMRefBoardV2.
 * It includes pins for alarm, in-position, current sensor, voltage sensor, and scaling factors.
 */
typedef struct
{
  int alarmPin;           /*> Pin connected to the ALM input */
  int inPositionPin;      /*> Pin connected to the PED input */
  int ADCPinCurrent;      /*> Pin connected to the current sensor input */
  float AmperePermV;      /*> Scaling of the current sensor ADC */
  int AmpereOffsetInmV;   /*> Offset in mV of the current sensor ADC */
  float currentThreshold; /*> Current threshold to count as hitting into an mechanical stop */
  int ADCPinVoltage;      /*> Pin connected to the voltage sensor input */
  float VoltPermV;        /*> Scaling of the voltage ADC */
} OSSMRefBoardV2Properties;

/**
 * @brief Class representing the motor control for OSSMRefBoardV2.
 *
 * This class provides methods for controlling the motor and retrieving motor properties.
 */
class OSSMRefBoardV2Motor : public GenericStepperMotor
{
public:
  /**
   * @brief Initializes the motor control.
   *
   * @param motor Pointer to the motor properties.
   * @param OSSMRefBoardV2 Pointer to the OSSMRefBoardV2 properties.
   */
  void begin(motorProperties *motor, OSSMRefBoardV2Properties *OSSMRefBoardV2);

  /**
   * @brief Measures the length of the rail.
   *
   * This measures the length of the rail by moving the motor back and forth until the in-position signal is reached on both sides.
   * The length is then calculated from the number of steps and the steps per millimeter and stored in the motor properties using setMachineGeometry().
   * @param keepout This keepout [mm] is a soft endstop and subtracted at both ends of the travel. A typical value would be 5mm.
   */
  void measureRailLength(float keepout = 5.0);

  /**
   * @brief Gets the current value.
   *
   * @param samples Number of samples to average.
   * @return The current in A.
   */
  float getCurrent(int samples = 20);

  /**
   * @brief Gets the voltage value.
   *
   * @param samples Number of samples to average.
   * @return The voltage in V.
   */
  float getVoltage(int samples = 20);

  /**
   * @brief Gets the power value.
   *
   * @param samples Number of samples to average.
   * @return The power in W.
   */
  float getPower(int samples = 20);

  /**************************************************************************/
  /*!
    @brief  It also attaches a callback function where the speed and position
    are reported on a regular interval specified with timeInMs.
    @param cbMotionPoint Callback with the signature
    `cbMotionPoint(unsigned int timestamp, float position, float speed)`. time is reported
    milliseconds since the controller has started (`millis()`), speed in [m/s] and
    position in [mm], voltage in [V] and current in [A].
    @param timeInMs time interval at which speed and position should be
    reported in [ms]
  */
  /**************************************************************************/
  void attachPositionFeedback(void (*cbMotionPoint)(unsigned int, float, float, float, float), unsigned int timeInMs = 20);

protected:
  /**
   * @brief Queries the home position.
   *
   * This function is repeatedly called during homing to check if the home position is reached. If no end stop switch is available,
   * this function will use the current sensor to detect the mechanical stop.
   *
   * @param samples Number of samples to average.
   * @return True if the home position is reached, false otherwise.
   */
  bool _queryHome(int samples = 20);

  /**
   * @brief Measure the motors idle current.
   */
  void _beginHoming();

  void _atHome() {}

private:
  OSSMRefBoardV2Properties *_OSSMRefBoardV2;

  /**
   * @brief Static method used as a callback for the measure procedure.
   */
  static void _measureProcedureImpl(void *_this) { static_cast<OSSMRefBoardV2Motor *>(_this)->_measureProcedure(); }

  /**
   * @brief Performs the measure procedure.
   */
  void _measureProcedure();

  /**************************************************************************/
  /*!
    @brief  A function that calls the motion point callback function
  */
  /**************************************************************************/
  void _callBackMotionPoint();

  /**
   * @brief Store the idle current of the motor when sitting idle with no external forces applied.
   */
  float _idleCurrent;

  TaskHandle_t _taskMeasuringHandle = NULL;
  void (*_cbMotionPoint)(unsigned int, float, float, float, float) = NULL;
};
