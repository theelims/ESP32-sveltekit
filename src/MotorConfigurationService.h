#pragma once

/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <StrokeEngine.h>
#include <NotificationService.h>

#include <StrokeEngineMotor.h>
#include <virtualMotor.h>
#include <genericStepper.h>
#include <OSSMRefBoardV2.h>

#ifdef OSSM_REF
#include <boards/OSSMReferenceBoard.h>
// else throw compile error
#else
#error "No board defined"
#endif

#ifndef MOTION_FACTORY_TRAVEL
#define MOTION_FACTORY_TRAVEL 150.0
#endif

#ifndef MOTION_HOMING_SPEED
#define MOTION_HOMING_SPEED 5.0
#endif

#define MOTOR_CONFIG_FILE "/config/motorConfig.json"
#define MOTOR_CONFIG_PATH "/rest/motorConfig"

// enum for the different drivers that can be used
enum MotorDriver
{
    VIRTUAL,
    GENERIC_STEPPER,
    OSSM_REF_BOARD_V2,
    IHSV_SERVO_V6
};

static motorProperties genericMotorProperties{
    .enableActiveLow = ENABLE_ACTIVE_LOW,
    .stepPin = STEP_PIN,
    .directionPin = DIRECTION_PIN,
    .enablePin = ENABLE_PIN,
};

static OSSMRefBoardV2Properties OSSMMotorProperties{
    .enableActiveLow = ENABLE_ACTIVE_LOW,
    .stepPin = STEP_PIN,
    .directionPin = DIRECTION_PIN,
    .enablePin = ENABLE_PIN,
    .alarmPin = ALARM_PIN,
    .inPositionPin = IN_POSITION_PIN,
    .ADCPinCurrent = ADC_PIN_CURRENT,
    .AmperePermV = AMPERE_PER_MILLIVOLT,
    .AmpereOffsetInmV = AMPERE_OFFSET_IN_MILLIVOLT,
    .ADCPinVoltage = ADC_PIN_VOLTAGE,
    .VoltPermV = VOLT_PER_MILLIVOLT,
};

class MotorConfiguration
{
public:
    MotorDriver driver;
    int stepPerRev;
    int maxRPM;
    int maxAcceleration;
    int pulleyTeeth;
    boolean invertDirection;
    boolean measureTravel;
    boolean home;
    float travel;
    float keepout;
    float sensorlessTrigger = SENSORLESS_TRIGGER;

    static void read(MotorConfiguration &settings, JsonObject &root)
    {
        // translate the enum to a string
        switch (settings.driver)
        {
        case VIRTUAL:
            root["driver"] = "VIRTUAL";
            break;
        case GENERIC_STEPPER:
            root["driver"] = "GENERIC_STEPPER";
            break;
        case OSSM_REF_BOARD_V2:
            root["driver"] = "OSSM_REF_BOARD_V2";
            break;
        default:
            root["driver"] = "VIRTUAL";
            break;
        }

        // create a new array for patterns
        JsonArray drivers = root["driver_list"].to<JsonArray>();
        // add drivers
        drivers.add("VIRTUAL");
#ifdef DRIVER_GENERIC_STEPPER
        drivers.add("GENERIC_STEPPER");
#endif
#ifdef DRIVER_OSSM_REF_BOARD_V2
        drivers.add("OSSM_REF_BOARD_V2");
#endif

        root["steps_per_rev"] = settings.stepPerRev;
        root["max_rpm"] = settings.maxRPM;
        root["max_acceleration"] = settings.maxAcceleration;
        root["pulley_teeth"] = settings.pulleyTeeth;
        root["invert_direction"] = settings.invertDirection;
        root["measure_travel"] = settings.measureTravel;
        root["home"] = false;
        root["travel"] = settings.travel;
        root["keepout"] = settings.keepout;
        root["sensorless_trigger"] = settings.sensorlessTrigger;
    }

    static StateUpdateResult update(JsonObject &root, MotorConfiguration &settings)
    {
        settings.measureTravel = root["measure_travel"] | false;
        settings.home = root["home"] | false;

        // DO not read the rest of the settings if we are measuring travel or homing
        if (settings.measureTravel || settings.home)
        {
            return StateUpdateResult::CHANGED;
        }

        settings.stepPerRev = root["steps_per_rev"] | STEP_PER_REV;
        settings.maxRPM = root["max_rpm"] | MAX_RPM;
        settings.maxAcceleration = root["max_acceleration"] | MAX_ACCELERATION;
        settings.pulleyTeeth = root["pulley_teeth"] | PULLEY_TEETH;
        settings.invertDirection = root["invert_direction"] | INVERT_DIRECTION;
        settings.travel = root["travel"] | MOTION_FACTORY_TRAVEL;
        settings.keepout = root["keepout"] | KEEP_OUT;
        settings.sensorlessTrigger = root["sensorless_trigger"] | SENSORLESS_TRIGGER;

        // translate the string to an enum
        String driver = root["driver"] | "VIRTUAL";
        if (driver == "VIRTUAL")
        {
            settings.driver = VIRTUAL;
        }
#ifdef DRIVER_GENERIC_STEPPER
        else if (driver == "GENERIC_STEPPER")
        {
            settings.driver = GENERIC_STEPPER;
        }
#endif
#ifdef DRIVER_OSSM_REF_BOARD_V2
        else if (driver == "OSSM_REF_BOARD_V2")
        {
            settings.driver = OSSM_REF_BOARD_V2;
        }
#endif
        else
        {
            settings.driver = VIRTUAL;
        }
        return StateUpdateResult::CHANGED;
    }
};

class MotorConfigurationService : public StatefulService<MotorConfiguration>
{
public:
    MotorConfigurationService(StrokeEngine *strokeEngine, PsychicHttpServer *server, FS *fs, SecurityManager *securityManager, NotificationService *notification);
    void begin();

    String getDriverName();

private:
    HttpEndpoint<MotorConfiguration> _httpEndpoint;
    FSPersistence<MotorConfiguration> _fsPersistence;
    NotificationService *_notification;
    StrokeEngine *_strokeEngine;
    MotorInterface *_motor;
    MotorDriver _loadedDriver = VIRTUAL;

    void onConfigUpdated(String originId);
};