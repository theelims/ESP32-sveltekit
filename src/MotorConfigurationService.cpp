/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <MotorConfigurationService.h>

MotorConfigurationService::MotorConfigurationService(StrokeEngine *strokeEngine, PsychicHttp *server, FS *fs, SecurityManager *securityManager, NotificationEvents *notificationEvent) : _strokeEngine(strokeEngine),
                                                                                                                                                                                         _httpEndpoint(MotorConfiguration::read,
                                                                                                                                                                                                       MotorConfiguration::update,
                                                                                                                                                                                                       this,
                                                                                                                                                                                                       server,
                                                                                                                                                                                                       MOTOR_CONFIG_PATH,
                                                                                                                                                                                                       securityManager,
                                                                                                                                                                                                       AuthenticationPredicates::IS_ADMIN),
                                                                                                                                                                                         _fsPersistence(MotorConfiguration::read, MotorConfiguration::update, this, fs, MOTOR_CONFIG_FILE),
                                                                                                                                                                                         _notificationEvent(notificationEvent)

{
    // configure settings service update handler to update state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(originId); },
                     false);
}

void MotorConfigurationService::begin()
{
    _fsPersistence.readFromFS();
    ESP_LOGI("MotorConfigurationService", "Motor config loaded from FS");

    _httpEndpoint.begin();

    // Create instances before the switch statement
    GenericStepperMotor *genericMotorInstance = nullptr;
    OSSMRefBoardV2Motor *OSSMMotorInstance = nullptr;
    iHSVServoV6Motor *IHSVMotorInstance = nullptr;

    // load correct motor driver
    switch (_state.driver)
    {
    case GENERIC_STEPPER:
        ESP_LOGI("MotorConfigurationService", "Using GenericStepperMotor");
        genericMotorInstance = new GenericStepperMotor();
        genericMotorInstance->begin(&genericMotorProperties);
        genericMotorInstance->setSensoredHoming(ENDSTOP_PIN, INPUT_PULLUP, true, 0.0, MOTION_HOMING_SPEED);
        genericMotorInstance->setStepsPerMillimeter(_state.stepPerRev / (_state.pulleyTeeth * BELT_PITCH));
        genericMotorInstance->invertDirection(_state.invertDirection);
        _motor = static_cast<MotorInterface *>(genericMotorInstance);
        _loadedDriver = GENERIC_STEPPER;
        break;
    case OSSM_REF_BOARD_V2:
        ESP_LOGI("MotorConfigurationService", "Using OSSMRefBoardV2Motor");
        OSSMMotorInstance = new OSSMRefBoardV2Motor();
        OSSMMotorInstance->begin(&OSSMMotorProperties);
        OSSMMotorInstance->setSensorlessHoming(MAX_AMPERE * (_state.sensorlessTrigger / 100.0), MOTION_HOMING_SPEED);
        OSSMMotorInstance->setStepsPerMillimeter(_state.stepPerRev / (_state.pulleyTeeth * BELT_PITCH));
        OSSMMotorInstance->invertDirection(_state.invertDirection);
        _motor = static_cast<MotorInterface *>(OSSMMotorInstance);
        _loadedDriver = OSSM_REF_BOARD_V2;
        break;
    case IHSV_SERVO_V6:
        ESP_LOGI("MotorConfigurationService", "Using iHSVServoV6Motor");
        IHSVMotorInstance = new iHSVServoV6Motor();
        IHSVMotorInstance->begin(&iHSVV6MotorProperties);
        IHSVMotorInstance->setSensorlessHoming((int)_state.sensorlessTrigger, MOTION_HOMING_SPEED);
        IHSVMotorInstance->setStepsPerMillimeter(_state.stepPerRev / (_state.pulleyTeeth * BELT_PITCH));
        IHSVMotorInstance->invertDirection(_state.invertDirection);
        _motor = static_cast<MotorInterface *>(IHSVMotorInstance);
        _loadedDriver = IHSV_SERVO_V6;
        break;
    default: // is also case VIRTUAL
        ESP_LOGI("MotorConfigurationService", "Using VirtualMotor");
        _motor = new VirtualMotor();
        _motor->begin();
        _loadedDriver = VIRTUAL;
        break;
    }

    // set motor parameters
    _motor->setMaxSpeed((_state.maxRPM / 60.0) * _state.pulleyTeeth * BELT_PITCH);
    _motor->setMaxAcceleration(_state.maxAcceleration);
    _motor->setMachineGeometry(_state.travel, _state.keepout);

    // enable motor to use it
    _motor->enable();

    // home motor
    _motor->home([&]()
                 {
                     if (_motor->isHomed())
                     {
                         _notificationEvent->pushNotification("Motor homed", pushEvent::PUSHSUCCESS, millis());
                     }
                     else
                     {
                         _notificationEvent->pushNotification("Motor homing failed", pushEvent::PUSHERROR, millis());
                     } });

    // attach motor to stroke engine
    _strokeEngine->attachMotor(_motor);
    ESP_LOGI("MotorConfigurationService", "Motor attached to stroke engine");
}

void MotorConfigurationService::onConfigUpdated(String originId)
{
    ESP_LOGI("MotorConfigurationService", "Stop StrokeEngine: motor config updated by %s", originId.c_str());

    // Ignore if originId is "measurement" to prevent endless loop
    if (originId == "measurement")
        return;

    // stop stroke engine
    _strokeEngine->stopMotion();

    // wait for motor to stop
    while (_motor->motionCompleted() == false)
    {
        delay(100);
    }

    // check if motor needs to be homed and return
    if (_state.home)
    {
        ESP_LOGI("MotorConfigurationService", "Homing motor requested");
        _state.home = false;
        _motor->home([&]()
                     {
                         if (_motor->isHomed())
                         {
                             _notificationEvent->pushNotification("Motor homed", pushEvent::PUSHSUCCESS, millis());
                         }
                         else
                         {
                             _notificationEvent->pushNotification("Motor homing failed", pushEvent::PUSHERROR, millis());
                         } });
        return;
    }

    // if measure travel was requested do only that
    else if (_state.measureTravel)
    {
        ESP_LOGI("MotorConfigurationService", "Measuring travel requested");

        // check if _motor is of type OSSMRefBoardV2Motor
        if (_loadedDriver == OSSM_REF_BOARD_V2)
        {
            // cast _motor to OSSMRefBoardV2Motor
            OSSMRefBoardV2Motor *OSSMMotor = static_cast<OSSMRefBoardV2Motor *>(_motor);
            // set motor to full speed
            OSSMMotor->measureRailLength([&]()
                                         {
                                             _notificationEvent->pushNotification("Measured travel finished", pushEvent::PUSHSUCCESS, millis());

                                             // save travel to config
                                             update([&](MotorConfiguration &state)
                                                    {   state.travel = _motor->getTravel();
                                                        state.keepout = _motor->getKeepout();
                                                        state.measureTravel = false;
                                                        return StateUpdateResult::CHANGED; },
                                                    "measurement");
                                             // done
                                         },
                                         _state.keepout);
        }

        // check if _motor is of type iHSVServoV6Motor
        else if (_loadedDriver == IHSV_SERVO_V6)
        {
            // cast _motor to iHSVServoV6Motor
            iHSVServoV6Motor *IHSVMotor = static_cast<iHSVServoV6Motor *>(_motor);
            // set motor to full speed
            IHSVMotor->measureRailLength([&]()
                                         {
                                             _notificationEvent->pushNotification("Measured travel finished", pushEvent::PUSHSUCCESS, millis());

                                             // save travel to config
                                             update([&](MotorConfiguration &state)
                                                    {   state.travel = _motor->getTravel();
                                                        state.keepout = _motor->getKeepout();
                                                        state.measureTravel = false;
                                                        return StateUpdateResult::CHANGED; },
                                                    "measurement");
                                             // done
                                         },
                                         _state.keepout);
        }
        else
        {
            _notificationEvent->pushNotification("Rail measurement not supported by this motor driver", pushEvent::PUSHERROR, millis());
            ESP_LOGW("MotorConfigurationService", "Rail measurement not supported by this motor driver");
            _state.measureTravel = false;
        }

        return;
    }
    else
    {
        // restart ESP32 to load changed motor driver
        ESP_LOGI("MotorConfigurationService", "Restarting ESP32 to load new motor driver");
        _motor->disable();
        delay(500);
        ESP.restart();
    }
}

String MotorConfigurationService::getDriverName()
{
    switch (_loadedDriver)
    {
    case VIRTUAL:
        return "VIRTUAL";
    case GENERIC_STEPPER:
        return "GENERIC_STEPPER";
    case OSSM_REF_BOARD_V2:
        return "OSSM_REF_BOARD_V2";
    case IHSV_SERVO_V6:
        return "IHSV_SERVO_V6";
    default:
        return "VIRTUAL";
    }
}
