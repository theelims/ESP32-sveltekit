/**
 *   Motor Driver for the OSSM Reference Board V2 of StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 * Copyright (C) 2023 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#pragma once

#include <Arduino.h>
#include <FastAccelStepper.h>
#include "ModbusClientRTU.h"

#include "motor.h"

/**************************************************************************/
/*!
  @brief  Monitoring register map of the iHSV V6 Servo Driver
*/
/**************************************************************************/
#define IHSVV6_MON_POSCMD 0x0834
#define IHSVV6_MON_POSACT 0x0836
#define IHSVV6_MON_POSERR 0x0838
#define IHSVV6_MON_TORQCMD 0x0843
#define IHSVV6_MON_TORQACT 0x0844

/**************************************************************************/
/*!
  @brief  P00 register map of the iHSV V6 Servo Driver
  General Motor Informations
*/
/**************************************************************************/
#define IHSVV6_P0001_RATEDRPM 0x0001
#define IHSVV6_P0002_RATEDTORQUE 0x0002
#define IHSVV6_P0003_RATEDCURRENT 0x0003
#define IHSVV6_P0021_RS232BAUDRATE 0x0015
#define IHSVV6_P0023_ADDRESS 0x0017
#define IHSVV6_P0024_MODBUSBAUD 0x0018
#define IHSVV6_P0025_MODBUSPARITY 0x0019

/**************************************************************************/
/*!
  @brief  P01 register map of the iHSV V6 Servo Driver
  Main Control Parameter
*/
/**************************************************************************/
#define IHSVV6_P0101_CONTROLMODE 0x0065
#define IHSVV6_P0102_AUTOTUNE 0x0066
#define IHSVV6_P0103_RIGIDITY 0x0067

/**************************************************************************/
/*!
  @brief  P03 register map of the iHSV V6 Servo Driver
  Position Parameter
*/
/**************************************************************************/
#define IHSVV6_P0305_INPOSMODE 0x0131
#define IHSVV6_P0306_INPOSMARGIN 0x0132
#define IHSVV6_P0315_DEVERROR 0x013B

/**************************************************************************/
/*!
  @brief  P05 register map of the iHSV V6 Servo Driver
  Position Parameter
*/
/**************************************************************************/
#define IHSVV6_P0500_TORQUECMDSOURCE 0x01F4
#define IHSVV6_P0505_TORQUELIMITSOURCE 0x01F9
#define IHSVV6_P0510_FWDTORQUELIMIT 0x01FE
#define IHSVV6_P0511_REVTORQUELIMIT 0x01FF

/**************************************************************************/
/*!
  @brief  Struct defining the motor (stepper or servo with STEP/DIR
  interface) and the motion system translating the rotation into a
  linear motion.This structure also contains the configuration parameters
  for the iHSVServoV6. It includes pins for alarm, in-position, current
  sensor, voltage sensor, and scaling factors.
*/
/**************************************************************************/
typedef struct
{
  int stepsPerMillimeter; /*> Number of steps per millimeter */
  bool invertDirection;   /*> Set to true to invert the direction signal
                           *  The firmware expects the home switch to be located at the
                           *  end of an retraction move. That way the machine homes
                           *  itself away from the body. Home position is -KEEPOUTBOUNDARY */
  bool enableActiveLow;   /*> Polarity of the enable signal. True for active low. */
  int stepPin;            /*> Pin connected to the STEP input */
  int directionPin;       /*> Pin connected to the DIR input */
  int enablePin;          /*> Pin connected to the ENA input */
  int alarmPin;           /*> Pin connected to the ALM input */
  int inPositionPin;      /*> Pin connected to the PED input */
  int modbusRxPin;        /*> Pin connected to the MODBUS RX input */
  int modbusTxPin;        /*> Pin connected to the MODBUS TX input */
  int torqueThreshold;    /*> Torque threshold for homing */
} iHSVServoV6Properties;

/**************************************************************************/
/*!
  @brief  iHSVServoV6 Motor inherits from MotorInterface and provides a generic
  STEP/DIR interface to all common stepper and servo drivers. Under the hood
  it uses FastAccelStepper for the trapezoidal motion planning and the
  hardware step signal generation. The generic stepper class uses the built-in
  current sensor for homing.
*/
/**************************************************************************/
class iHSVServoV6Motor : public MotorInterface
{
public:
  /**************************************************************************/
  /*!
   * @brief Initializes the motor control.
   *
   * @param motor Pointer to the motor properties.
   * @param iHSVServoV6 Pointer to the iHSVServoV6 properties.
   */
  /**************************************************************************/
  void begin(iHSVServoV6Properties *iHSVServoV6)
  {
    _motor = iHSVServoV6;

    // Establish Modbus connection to servo
    RTUutils::prepareHardwareSerial(Serial2);
    Serial2.begin(57600, SERIAL_8E1, _motor->modbusRxPin, _motor->modbusTxPin);
    _MB.setTimeout(2000);
    _MB.begin(Serial2);

    // Setup FastAccelStepper
    engine.init();
    _stepper = engine.stepperConnectToPin(_motor->stepPin);
    if (_stepper)
    {
      _stepper->setDirectionPin(_motor->directionPin, _motor->invertDirection);
      _stepper->setEnablePin(_motor->enablePin, _motor->enableActiveLow);
      _stepper->setAutoEnable(false);
      _stepper->disableOutputs();
      ESP_LOGI("iHSVServoV6", "FastAccelStepper Engine successfully initialized!");
    }
    else
    {
      ESP_LOGE("iHSVServoV6", "Failed to load FastAccelStepper Engine!");
    }

    pinMode(_motor->alarmPin, INPUT_PULLUP);
    pinMode(_motor->inPositionPin, INPUT_PULLUP);

    int servoRatedRPM = _readServoRegister(IHSVV6_P0001_RATEDRPM);
    int servoRatedTorque = _readServoRegister(IHSVV6_P0002_RATEDTORQUE);
    int servoRatedCurrent = _readServoRegister(IHSVV6_P0003_RATEDCURRENT);
    int servoFwdTorqueLimit = _readServoRegister(IHSVV6_P0510_FWDTORQUELIMIT);
    int servoRevTorqueLimit = _readServoRegister(IHSVV6_P0511_REVTORQUELIMIT, true);
    // Log all servo stats to serial
    ESP_LOGI("iHSVServoV6", "Servo Stats: rated RPM: %d, rated torque: %d, rated current: %d, fwd torque limit: %d, rev torque limit: %d",
             servoRatedRPM,
             servoRatedTorque,
             servoRatedCurrent,
             servoFwdTorqueLimit,
             servoRevTorqueLimit);

    // write default settings to servo (temporary)
    _writeServoRegister(IHSVV6_P0305_INPOSMODE, 0);
    _writeServoRegister(IHSVV6_P0306_INPOSMARGIN, 10);
    _writeServoRegister(IHSVV6_P0315_DEVERROR, 65535);
    _writeServoRegister(IHSVV6_P0103_RIGIDITY, 20);
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void setMachineGeometry(float travel, float keepout = 5.0)
  {
    _travel = travel;
    _keepout = keepout;
    _maxPosition = travel - (keepout * 2);
    _minStep = 0;
    _maxStep = int(0.5 + _maxPosition * _motor->stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _motor->stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _motor->stepsPerMillimeter);
    ESP_LOGD("iHSVServoV6", "Machine Geometry Travel = %f", _travel);
    ESP_LOGD("iHSVServoV6", "Machine Geometry Keepout = %f", _keepout);
    ESP_LOGD("iHSVServoV6", "Machine Geometry MaxPosition = %f", _maxPosition);
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overall mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void home(float homePosition = 0.0, float speed = 5.0)
  {
    _homePosition = int(0.5 + homePosition / float(_motor->stepsPerMillimeter));
    _homingSpeed = speed * _motor->stepsPerMillimeter;
    ESP_LOGI("iHSVServoV6", "Search home with %05.1f mm/s at %05.1f mm.", speed, homePosition);

    // set homed to false so that isActive() becomes false
    _homed = false;

    // first stop current motion and suspend motion tasks
    stopMotion();

    // Quit if stepper not enabled
    if (_enabled == false)
    {
      ESP_LOGE("iHSVServoV6", "Homing not possible! --> Enable stepper first!");
      return;
    }

    // Create homing task
    xTaskCreatePinnedToCore(
        this->_homingProcedureImpl, // Function that should be called
        "Homing",                   // Name of the task (for debugging)
        4096,                       // Stack size (bytes)
        this,                       // Pass reference to this class instance
        1,                          // Pretty high task priority
        &_taskHomingHandle,         // Task handle
        1                           // Have it on application core
    );
    ESP_LOGD("iHSVServoV6", "Created Homing Task.");
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void home(void (*callBackHoming)(bool), float homePosition = 0.0, float speed = 5.0)
  {
    _callBackHoming = callBackHoming;
    home(homePosition, speed);
  }

  /**************************************************************************/
  /*!
   * @brief Measures the length of the rail.
   *
   * This measures the length of the rail by moving the motor back and forth until the in-position signal is reached on both sides.
   * The length is then calculated from the number of steps and the steps per millimeter and stored in the motor properties using setMachineGeometry().
   * @param keepout This keepout [mm] is a soft endstop and subtracted at both ends of the travel. A typical value would be 5mm.
   */
  /**************************************************************************/
  void measureRailLength(float keepout = 5.0)
  {
    // Quit if stepper not enabled
    if (_enabled == false)
    {
      ESP_LOGE("iHSVServoV6", "Measuring not possible! --> Enable stepper first!");
      return;
    }

    // first stop current motion and suspend motion tasks
    stopMotion();

    // store the keepout distance
    _keepout = keepout;

    ESP_LOGI("iHSVServoV6", "Measuring rail length...");

    // create a one-time task to perform the measure procedure
    xTaskCreatePinnedToCore(
        this->_measureProcedureImpl, // Function that should be called
        "Measuring",                 // Name of the task (for debugging)
        4096,                        // Stack size (bytes)
        this,                        // Pass reference to this class instance
        1,                           // Pretty high task priority
        &_taskMeasuringHandle,       // Task handle
        1                            // Have it on application core
    );
    ESP_LOGD("iHSVServoV6", "Created Homing Task.");
  }

  /**************************************************************************/
  /*!
    @brief  It also attaches a callback function where the speed and position
    are reported on a regular interval specified with timeInMs.
    @param cbMotionPoint Callback with the signature
    `cbMotionPoint(unsigned int timestamp, float position, float speed)`. time is reported
    milliseconds since the controller has started (`millis()`), speed in [m/s] and
    position in [mm].
    @param timeInMs time interval at which speed and position should be
    reported in [ms]
  */
  /**************************************************************************/
  void attachPositionFeedback(void (*cbMotionPoint)(unsigned int, float, float, float, float), unsigned int timeInMs = 50)
  {
    _cbMotionPoint = cbMotionPoint;
    _timeSliceInMs = timeInMs / portTICK_PERIOD_MS;
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overall mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void enable()
  {
    ESP_LOGI("iHSVServoV6", "Stepper Enabled!");
    // Enable stepper
    _enabled = true;
    _stepper->enableOutputs();

    if (_cbMotionPoint == NULL)
    {
      ESP_LOGD("iHSVServoV6", "No Position Feedback Task created.");
      return;
    }

    // Create / resume motion feedback task
    if (_taskPositionFeedbackHandle == NULL)
    {
      // Create Stroke Task
      xTaskCreatePinnedToCore(
          _positionFeedbackTaskImpl,    // Function that should be called
          "Motion Feedback",            // Name of the task (for debugging)
          4096,                         // Stack size (bytes)
          this,                         // Pass reference to this class instance
          10,                           // Pretty high task priority
          &_taskPositionFeedbackHandle, // Task handle
          1                             // Pin to application core
      );
      ESP_LOGD("iHSVServoV6", "Created Position Feedback Task.");
    }
    else
    {
      // Resume task, if it already exists
      vTaskResume(_taskPositionFeedbackHandle);
      ESP_LOGD("iHSVServoV6", "Resumed Position Feedback Task.");
    }
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overall mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void disable()
  {
    ESP_LOGI("iHSVServoV6", "Stepper Disabled!");
    // Disable stepper
    _enabled = false;
    _stepper->disableOutputs();

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
      vTaskDelete(_taskHomingHandle);
      _taskHomingHandle = NULL;
      ESP_LOGD("iHSVServoV6", "Deleted Homing Task.");
    }

    // Suspend motion feedback task if it exists already
    if (_taskPositionFeedbackHandle != NULL)
    {
      vTaskSuspend(_taskPositionFeedbackHandle);
      ESP_LOGD("iHSVServoV6", "Suspended Position Feedback Task.");
    }
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void stopMotion()
  {

    ESP_LOGW("iHSVServoV6", "STOP MOTION!");

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
      vTaskDelete(_taskHomingHandle);
      _taskHomingHandle = NULL;
      ESP_LOGD("iHSVServoV6", "Deleted Homing Task.");
    }

    if (_stepper->isRunning())
    {
      // Stop servo motor as fast as legally allowed
      _stepper->setAcceleration(_maxStepAcceleration);
      _stepper->applySpeedAcceleration();
      _stepper->stopMove();
      ESP_LOGD("iHSVServoV6", "Bring stepper to a safe halt.");
    }

    // Wait for servo stopped
    while (_stepper->isRunning())
      ;
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  bool motionCompleted() { return _stepper->isRunning() ? false : true; }

  /**************************************************************************/
  /*!
    @brief  Returns the currently used acceleration.
    @return acceleration of the motor in [mm/s²]
  */
  /**************************************************************************/
  float getAcceleration() { return float(_stepper->getAcceleration()) / float(_motor->stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current speed the machine.
    @return speed of the motor in [mm/s]
  */
  /**************************************************************************/
  float getSpeed() { return (float(_stepper->getCurrentSpeedInMilliHz()) * 1.0e-3) / float(_motor->stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current position of the machine.
    @return position in [mm]
  */
  /**************************************************************************/
  float getPosition() { return float(_stepper->getCurrentPosition()) / float(_motor->stepsPerMillimeter); }

private:
  /**************************************************************************/
  /*!
    @brief  Internal function that updates the trapezoidal motion path
    generator. Here this is done by calling the appropriate FastAccelStepper
    API calls and translate between metric units and steps
    @param position in [mm]
    @param speed in [mm/s]
    @param acceleration in [mm/s²]
  */
  /**************************************************************************/
  void _unsafeGoToPosition(float position, float speed, float acceleration)
  {
    // Translate between metric and steps
    int speedInHz = int(0.5 + speed * _motor->stepsPerMillimeter);
    int stepAcceleration = int(0.5 + acceleration * _motor->stepsPerMillimeter);
    int positionInSteps = int(0.5 + position * _motor->stepsPerMillimeter);
    ESP_LOGD("iHSVServoV6", "Going to unsafe position %i steps @ %i steps/s, %i steps/s^2", positionInSteps, speedInHz, stepAcceleration);

    // write values to stepper
    _stepper->setSpeedInHz(speedInHz);
    _stepper->setAcceleration(stepAcceleration);
    _stepper->moveTo(positionInSteps);
  }

  /**************************************************************************/
  /*!
    @brief  Read 16bit values from the servo register map.
    @param address Address of the register to read.
    @param sign Set to true if the register is signed.
    @return value of the register.
  */
  /**************************************************************************/
  int _readServoRegister(uint16_t address, boolean sign = false)
  {
    ModbusMessage response = _MB.syncRequest(token++, 1, READ_HOLD_REGISTER, address, 1);
    Error err = response.getError();
    if (err != SUCCESS)
    {
      ModbusError e(err);
      ESP_LOGE("iHSVServoV6", "Error creating Modbus request at %#04x: %02X - %s", address, (int)e, (const char *)e);
    }
    else
    {
      uint16_t value;
      int result;
      response.get(3, value);

      if (sign && value > 32676)
      {
        result = value - 65536;
      }
      else
      {
        result = value;
      }
      ESP_LOGV("iHSVServoV6", "Modbus read at %#04x: FC=%d, length=%d, value= %d", address, response.getFunctionCode(), response[2], result);
      return result;
    }
    return 0;
  }

  /**************************************************************************/
  /*!
    @brief  Read 32bit values from the servo register map.
    @param address Address of the register to read.
    @param sign Set to true if the register is signed.
    @return value of the register.
  */
  /**************************************************************************/
  int _readServoRegister32bit(uint16_t address, boolean sign = false)
  {
    ModbusMessage response = _MB.syncRequest(token++, 1, READ_HOLD_REGISTER, address, 2);
    Error err = response.getError();
    if (err != SUCCESS)
    {
      ModbusError e(err);
      ESP_LOGE("iHSVServoV6", "Error creating Modbus request at %#04x: %02X - %s", address, (int)e, (const char *)e);
    }
    else
    {
      uint32_t value;
      int result;
      response.get(3, value);

      if (sign && value > 2147482647)
      {
        result = value - 4294967296;
      }
      else
      {
        result = value;
      }
      ESP_LOGV("iHSVServoV6", "Modbus 32bit read at %#04x: FC=%d, length=%d, value=%d", address, response.getFunctionCode(), response[2], result);
      return result;
    }
    return 0;
  }

  /**************************************************************************/
  /*!
    @brief  Write values to the servo register map.
    @param address Address of the register to write.
    @param writeValue Value to write to the register.
    @param sign Set to true if the register is signed.
    @param permanent Set to true if the register should be written permanently.
    @return true if the write was successful.
  */
  /**************************************************************************/
  boolean _writeServoRegister(uint16_t address, int writeValue, boolean sign = false, boolean permanent = false)
  {
    // add 0x8000 to address to store values permanent
    if (permanent)
    {
      address = address + 0x8000;
    }

    // return false if negative value is written to unsigned register
    if (!sign && writeValue < 0)
    {
      ESP_LOGE("iHSVServoV6", "Error writing to Modbus register: Negative value written to unsigned register.");
      return false;
    }

    // convert negative values to 2's complement
    if (sign && writeValue < 0)
    {
      writeValue = 65536 - abs(writeValue);
    }

    ModbusMessage response = _MB.syncRequest(token++, 1, WRITE_HOLD_REGISTER, address, writeValue);
    Error err = response.getError();
    if (err != SUCCESS)
    {
      ModbusError e(err);
      ESP_LOGE("iHSVServoV6", "Error creating Modbus request write at %#04x: %02X - %s", address, (int)e, (const char *)e);
      return false;
    }

    ESP_LOGV("iHSVServoV6", "Modbus write at %#04x: FC=%d, written=%d", address, response.getFunctionCode(), writeValue);
    return true;
  }

  bool _queryHome()
  {
    // read actual torque from servo
    int actualTorque = _readServoRegister(IHSVV6_MON_TORQACT, true);
    ESP_LOGD("iHSVServoV6", "Actual Absolute Torque: %d", actualTorque);
    if (abs(actualTorque) > _motor->torqueThreshold)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  void _homingProcedure()
  {
    ESP_LOGD("iHSVServoV6", "Start searching for home.");

    if (_stepper->isRunning())
    {
      ESP_LOGD("iHSVServoV6", "Stepper is moving. Stop and home");
      stopMotion();
    }

    // Set feedrate for homing
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration / 10);

    // Move towards the home position
    _stepper->runBackward();

    // Poll homing switch
    while (_stepper->isRunning())
    {

      // Are we at the home position?
      if (_queryHome())
      {
        ESP_LOGD("iHSVServoV6", "Found home!");
        // Set home position
        // Switch is at -KEEPOUT
        _stepper->forceStopAndNewPosition(_motor->stepsPerMillimeter * int(_homePosition - _keepout));

        // drive free of switch and set axis to lower end
        _stepper->moveTo(_minStep);

        _homed = true;

        // drive free of switch and set axis to 0
        _stepper->moveTo(0);

        // Break loop, home was found
        break;
      }

      // Pause the task for 20ms to allow other tasks
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // disable Servo if homing has not found the homing switch
    if (!_homed)
    {
      _stepper->disableOutputs();
      ESP_LOGE("iHSVServoV6", "Homing failed! Did not find home position.");
    }

    // Call notification callback, if it was defined.
    if (_callBackHoming != NULL)
    {
      _callBackHoming(_homed);
    }

    // delete one-time task
    _taskHomingHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("iHSVServoV6", "Homing task self-terminated");
  }

  void _measureProcedure()
  {
    // home the motor
    home();

    // wait until the motor is in position
    while (!_homed)
    {
      // Pause the task for 20ms to allow other tasks
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // measure the rail length
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration);

    // move motor into the other direction
    _stepper->runForward();

    // wait until the motor is in position
    while (!_motionCompleted)
    {
      // query endstop
      if (_queryHome())
      {
        // stop the motor
        _stepper->stopMove();

        // set the current position as the travel length. Add one keepout to account for homing on one side.
        float travel = getPosition() + _keepout;
        ESP_LOGI("iHSVServoV6", "Measured rail length: %f", travel);
        setMachineGeometry(travel, _keepout);

        // move the motor back to the home position
        _stepper->move(_motor->stepsPerMillimeter * (_maxPosition - _keepout));
      }

      // Pause the task for 20ms to allow other tasks
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // delete one-time task
    _taskMeasuringHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("iHSVServoV6", "Measuring task self-terminated");
  }

  void _positionFeedbackTask()
  {
    TickType_t xLastWakeTime;
    // Initialize the xLastWakeTime variable with the current tick count.
    xLastWakeTime = xTaskGetTickCount();

    unsigned int now = millis();

    while (true)
    {
      // Return results of current motion point via the callback
      /*       _cbMotionPoint(
                millis(),
                getPosition(),
                getSpeed(),
                0.0,
                0.0); */

      // read positions from servo
      int commandPosition = _readServoRegister32bit(IHSVV6_MON_POSCMD, true);
      int actualPosition = _readServoRegister32bit(IHSVV6_MON_POSACT, true);
      int positionError = _readServoRegister32bit(IHSVV6_MON_POSERR, true);

      // read torques from servo
      int commandTorque = _readServoRegister(IHSVV6_MON_TORQCMD, true);
      int actualTorque = _readServoRegister(IHSVV6_MON_TORQACT, true);

      // Read pins of the servo
      int alarm = digitalRead(_motor->alarmPin);
      int inPosition = digitalRead(_motor->inPositionPin);

      // Print results on Serial
      Serial.printf("Time: %d, Command Position: %d, Actual Position: %d, Position Error: %d, Command Torque: %d, Actual Torque: %d, Alarm: %d, In Position: %d\n",
                    millis(),
                    commandPosition,
                    actualPosition,
                    positionError,
                    commandTorque,
                    actualTorque,
                    alarm,
                    inPosition);

      // Delay the task until the next tick count
      vTaskDelayUntil(&xLastWakeTime, _timeSliceInMs);
    }
  }

  iHSVServoV6Properties *_motor;
  FastAccelStepper *_stepper;
  ModbusClientRTU _MB;
  FastAccelStepperEngine engine = FastAccelStepperEngine();
  float _idleCurrent;
  int _minStep;
  int _maxStep;
  int _maxStepPerSecond;
  int _maxStepAcceleration;
  bool _motionCompleted = true;
  static void _homingProcedureImpl(void *_this) { static_cast<iHSVServoV6Motor *>(_this)->_homingProcedure(); }
  int _homingSpeed;
  float _homePosition;
  TaskHandle_t _taskHomingHandle = NULL;
  void (*_callBackHoming)(bool) = NULL;
  TickType_t _timeSliceInMs = 50;
  static void _positionFeedbackTaskImpl(void *_this) { static_cast<iHSVServoV6Motor *>(_this)->_positionFeedbackTask(); }
  TaskHandle_t _taskPositionFeedbackHandle = NULL;
  static void _measureProcedureImpl(void *_this) { static_cast<iHSVServoV6Motor *>(_this)->_measureProcedure(); }
  TaskHandle_t _taskMeasuringHandle = NULL;
  void (*_cbMotionPoint)(unsigned int, float, float, float, float) = NULL;
  unsigned int token = 1111;
};
