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
  bool enableActiveLow; /*> Polarity of the enable signal. True for active low. */
  int stepPin;          /*> Pin connected to the STEP input */
  int directionPin;     /*> Pin connected to the DIR input */
  int enablePin;        /*> Pin connected to the ENA input */
  int alarmPin;         /*> Pin connected to the ALM input */
  int inPositionPin;    /*> Pin connected to the PED input */
  int modbusRxPin;      /*> Pin connected to the MODBUS RX input */
  int modbusTxPin;      /*> Pin connected to the MODBUS TX input */
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
  iHSVServoV6Motor() : MotorInterface() // Constructor
  {
    // flag error as true until we have proven the opposite
    _error = true;
  };
  /**************************************************************************/
  /*!
   @brief Initializes the motor control.

   @param iHSVServoV6 Pointer to the iHSVServoV6 properties.
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
      _stepper->setDirectionPin(_motor->directionPin, !_invertDirection);
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

    _servoRatedTorque = (float)_readServoRegister(IHSVV6_P0002_RATEDTORQUE) * 0.01;
    // Log all servo stats to serial
    ESP_LOGI("iHSVServoV6", "Servo rated torque: %f Nm", _servoRatedTorque);

    // write default settings to servo (temporary)
    if (!_writeServoRegister(IHSVV6_P0315_DEVERROR, 65535))
    {
      ESP_LOGE("iHSVServoV6", "Error to access servo over Modbus. Check wiring and try again.");
      _error = true;
    }
    else
    {
      _error = false;
      ESP_LOGI("iHSVServoV6", "Servo successfully initialized!");
    }
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines steps per millimeter of travel. This is used
    to translate between metric units and steps.
    @param  stepsPerMillimeter steps per millimeter of travel.
  */
  /**************************************************************************/
  void setStepsPerMillimeter(int stepsPerMillimeter = 50)
  {
    _stepsPerMillimeter = stepsPerMillimeter;
    _maxStep = int(0.5 + _maxPosition * _stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _stepsPerMillimeter);
    ESP_LOGD("iHSVServoV6", "Steps per millimeter set to %d", _stepsPerMillimeter);
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
    _maxStep = int(0.5 + _maxPosition * _stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _stepsPerMillimeter);
    ESP_LOGD("iHSVServoV6", "Machine Geometry Travel = %f", _travel);
    ESP_LOGD("iHSVServoV6", "Machine Geometry Keepout = %f", _keepout);
    ESP_LOGD("iHSVServoV6", "Machine Geometry MaxPosition = %f", _maxPosition);
  }

  /**************************************************************************/
  /*!
    @brief  Sets up sensorless homing.
    @param threshold Torque threshold to account as mechanical endstop.
  */
  /**************************************************************************/
  void setSensorlessHoming(int threshold = 40)
  {
    _torqueThreshold = threshold * 10;
  }

  /**************************************************************************/
  /*!
    @brief  Changes the rigidity (stiffness) tuning of the servo. The value
    is given in [0-31]. A value of 0 means the servo is very elastic and will
    give in easily. A value of 31 means the servo is very stiff. Effect is only
    temporary. After a power cycle the servo will return to its default value.
    @param rigidity rigidity of the servo in [0-31]. Default tuning is 13.
  */
  /**************************************************************************/
  void setRigidity(int rigidity = 13)
  {
    rigidity = constrain(rigidity, 0, 31);
    _writeServoRegister(IHSVV6_P0103_RIGIDITY, rigidity);
  }

  /**************************************************************************/
  /*!
    @brief Homes the machine. This is done by moving the motor towards the
    homing switch until the torque threshold is reached. The homing switch
    is then set as the new home position.
    @param homePosition Position of the homing switch in [mm]. Default is 0.0.
    @param speed Speed of the homing procedure in [mm/s]. Default is 5.0.
  */
  /**************************************************************************/
  void home(float homePosition = 0.0, float speed = 5.0)
  {
    if (_error)
    {
      ESP_LOGE("iHSVServoV6", "Homing not possible! --> Servo in error state!");
      return;
    }

    _homePosition = int(0.5 + homePosition / float(_stepsPerMillimeter));
    _homingSpeed = speed * _stepsPerMillimeter;
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
    @brief Homes the machine. This is done by moving the motor towards the
    homing switch until the current threshold is reached. The homing switch
    is then set as the new home position.
    @param callBackHoming Callback function that is called when homing is
    completed. The callback function must take a boolean as an argument.
    @param homePosition Position of the homing switch in [mm]. Default is 0.0.
    @param speed Speed of the homing procedure in [mm/s]. Default is 5.0.
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
   * This measures the length of the rail by moving the motor back and forth until the endstop is reached on both sides.
   * The length is then calculated from the number of steps and the steps per millimeter and stored in the motor properties using setMachineGeometry().
   * @param callBackMeasuring Callback function that is called when measuring is completed. The callback function must take a float as an argument.
   * @param keepout This keepout [mm] is a soft endstop and subtracted at both ends of the travel. A typical value would be 5mm.
   */
  /**************************************************************************/
  void measureRailLength(void (*callBackMeasuring)(float), float keepout = 5.0)
  {
    if (_error)
    {
      ESP_LOGE("iHSVServoV6", "Measuring not possible! --> Servo in error state!");
      return;
    }

    // store the call back function
    _callBackMeasuring = callBackMeasuring;

    // Quit if stepper not enabled
    if (_enabled == false)
    {
      ESP_LOGE("iHSVServoV6", "Measuring not possible! --> Enable stepper first!");
      return;
    }

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
    @brief Enables the stepper motor.
  */
  /**************************************************************************/
  void enable()
  {
    if (_error)
    {
      ESP_LOGE("iHSVServoV6", "Enable not possible! --> Servo in error state!");
      return;
    }

    ESP_LOGI("iHSVServoV6", "Stepper Enabled!");
    // Enable stepper
    _enabled = true;
    _stepper->enableOutputs();
  }

  /**************************************************************************/
  /*!
    @brief  Disable the stepper motor.
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
    @brief  Stops any motion as fast as legally possible.
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

    // Delete measuring task should the measuring sequence be running
    if (_taskMeasuringHandle != NULL)
    {
      vTaskDelete(_taskMeasuringHandle);
      _taskMeasuringHandle = NULL;
      ESP_LOGD("OSSMRefBoardV2", "Deleted Measuring Task.");
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
    @brief  Returns if a trapezoidal motion is carried out, or the machine is
    at stand-still.
    @return `true` if motion is completed, `false` if still under way
  */
  /**************************************************************************/
  bool motionCompleted() { return _stepper->isRunning() ? false : true; }

  /**************************************************************************/
  /*!
    @brief  Returns the currently used acceleration.
    @return acceleration of the motor in [mm/s²]
  */
  /**************************************************************************/
  float getAcceleration() { return float(_stepper->getAcceleration()) / float(_stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current speed the machine.
    @return speed of the motor in [mm/s]
  */
  /**************************************************************************/
  float getSpeed() { return (float(_stepper->getCurrentSpeedInMilliHz()) * 1.0e-3) / float(_stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current position of the machine.
    @return position in [mm]
  */
  /**************************************************************************/
  float getPosition() { return float(_stepper->getCurrentPosition()) / float(_stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the error state of the motor. This is `true` if the motor
    is in an error state and `false` if everything is fine.
    @return error state
  */
  /**************************************************************************/
  int hasError()
  {
    // Check if alarm is active
    if (digitalRead(_motor->alarmPin) == LOW || _error)
    {
      ESP_LOGE("OSSMRefBoardV2", "Alarm signal is active!");
      return 1;
    }
    return 0;
  }

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
    int speedInHz = int(0.5 + speed * _stepsPerMillimeter);
    int stepAcceleration = int(0.5 + acceleration * _stepsPerMillimeter);
    int positionInSteps = int(0.5 + position * _stepsPerMillimeter);
    ESP_LOGD("iHSVServoV6", "Going to unsafe position %i steps @ %i steps/s, %i steps/s^2", positionInSteps, speedInHz, stepAcceleration);

    // write values to stepper
    _stepper->setSpeedInHz(speedInHz);
    _stepper->setAcceleration(stepAcceleration);
    _stepper->moveTo(positionInSteps);
  }

  /**************************************************************************/
  /*!
    @brief  Read 16bit values from the servo register map. Retries up to 2x if
    a read fails. Returns 0 if all retries failed.
    @param address Address of the register to read.
    @param sign Set to true if the register is signed.
    @return value of the register.
  */
  /**************************************************************************/
  int _readServoRegister(uint16_t address, boolean sign = false)
  {
    for (size_t i = 0; i < 3; i++)
    {
      ModbusMessage response = _MB.syncRequest(token++, 1, READ_HOLD_REGISTER, address, 1);
      Error err = response.getError();
      if (err != SUCCESS)
      {
        ModbusError e(err);
        ESP_LOGD("iHSVServoV6", "Error creating Modbus request at %#04x: %02X - %s", address, (int)e, (const char *)e);
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
    }
    return 0;
  }

  /**************************************************************************/
  /*!
    @brief  Read 32bit values from the servo register map. Retries up to 2x if
    a read fails. Returns 0 if all retries failed.
    @param address Address of the register to read.
    @param sign Set to true if the register is signed.
    @return value of the register.
  */
  /**************************************************************************/
  int _readServoRegister32bit(uint16_t address, boolean sign = false)
  {
    for (size_t i = 0; i < 3; i++)
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
    ESP_LOGV("iHSVServoV6", "Actual Torque: %d", actualTorque);
    if (abs(actualTorque) > _torqueThreshold)
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
    ESP_LOGI("iHSVServoV6", "Start searching for home.");

    // Set feedrate for homing
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration);

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
        _stepper->forceStopAndNewPosition(_stepsPerMillimeter * int(_homePosition - _keepout));

        // store teh current servo position as offset
        _servoPositionOffset = _readServoRegister32bit(IHSVV6_MON_POSACT, true);
        ESP_LOGI("iHSVServoV6", "Servo position offset: %d", _servoPositionOffset);

        // drive free of switch and set axis to lower end
        _stepper->moveTo(_minStep);

        // drive free of switch and set axis to 0
        _stepper->moveTo(0, true); // blocking call

        _homed = true;

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
    float travel = 0.0;

    // home the motor
    home();

    // wait until the motor is in position
    while (!_homed)
    {
      // Pause the task for 100ms while waiting for home to complete
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    // measure the rail length
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration);

    // move motor into the other direction
    _stepper->runForward();

    // wait until the motor is in position
    while (_stepper->isRunning())
    {
      // query endstop
      if (_queryHome())
      {
        // stop the motor
        _stepper->stopMove();

        // set the current position as the travel length. Add one keepout to account for homing on one side.
        travel = getPosition() + _keepout;
        ESP_LOGI("iHSVServoV6", "Measured rail length: %f", travel);
        setMachineGeometry(travel, _keepout);

        // drive free of end to _maxPosition
        _stepper->moveTo(_maxStep);

        break;
      }

      // Pause the task for 20ms to allow other tasks
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // Call notification callback, if it was defined.
    if (_callBackMeasuring != NULL)
    {
      _callBackMeasuring(travel);
    }

    // delete one-time task
    _taskMeasuringHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("iHSVServoV6", "Measuring task self-terminated");
  }

  void _reportMotionPoint()
  {
    // read position from servo
    int actualPosition = _readServoRegister32bit(IHSVV6_MON_POSACT, true);

    // read torque from servo
    int actualTorque = _readServoRegister(IHSVV6_MON_TORQACT, true);

    // Read pins of the servo
    int alarm = digitalRead(_motor->alarmPin);

    // Return results of current motion point via the callback
    _cbMotionPoint(
        millis(),
        getPosition(),
        getSpeed(),
        (float)(actualPosition - _servoPositionOffset) / (float)_stepsPerMillimeter,
        (float)actualTorque * 0.1 * _servoRatedTorque);
  }

  iHSVServoV6Properties *_motor;
  FastAccelStepper *_stepper;
  ModbusClientRTU _MB;
  unsigned int token = 1111;
  FastAccelStepperEngine engine = FastAccelStepperEngine();
  int _stepsPerMillimeter = 50;
  int _minStep;
  int _maxStep;
  int _maxStepPerSecond;
  int _maxStepAcceleration;
  static void _homingProcedureImpl(void *_this) { static_cast<iHSVServoV6Motor *>(_this)->_homingProcedure(); }
  int _homingSpeed;
  float _homePosition;
  TaskHandle_t _taskHomingHandle = NULL;
  void (*_callBackHoming)(bool) = NULL;
  static void _measureProcedureImpl(void *_this) { static_cast<iHSVServoV6Motor *>(_this)->_measureProcedure(); }
  TaskHandle_t _taskMeasuringHandle = NULL;
  void (*_callBackMeasuring)(float) = NULL;
  int _torqueThreshold; /*> Torque threshold for homing */
  float _servoRatedTorque;
  int _servoPositionOffset = 0;
};
