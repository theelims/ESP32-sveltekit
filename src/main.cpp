/**
 *   LUST-Motion
 *
 *
 *
 *   https://github.com/openlust/LUST-motion
 *
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

// #define VIRTUAL

#include <ESP32SvelteKit.h>
#include <ESPmDNS.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include <StrokeEngine.h>
#include <SettingValue.h>
#include <StrokeEngineControlService.h>
#include <WebSocketRawDataStreaming.h>

#include <motor/virtualMotor.h>
#include <motor/genericStepper.h>
#include <motor/OSSMRefBoardV2.h>
#include <motor/ihsvServoV6.h>

/*#################################################################################################
##
##    G L O B A L    D E F I N I T I O N S   &   D E C L A R A T I O N S
##
##################################################################################################*/

// StrokeEngine ###################################################################################
// Calculation Aid:
#define STEP_PER_REV 2000 // How many steps per revolution of the motor (S1 off, S2 on, S3 on, S4 off)
#define PULLEY_TEETH 16   // How many teeth has the pulley
#define BELT_PITCH 2      // What is the timing belt pitch in mm
#define MAX_RPM 3000.0    // Maximum RPM of motor
#define STEP_PER_MM STEP_PER_REV / (PULLEY_TEETH * BELT_PITCH)
#define MAX_SPEED (MAX_RPM / 60.0) * PULLEY_TEETH *BELT_PITCH

static motorProperties genericMotorProperties{
    .stepsPerMillimeter = STEP_PER_MM,
    .invertDirection = true,
    .enableActiveLow = true,
    .stepPin = 14,
    .directionPin = 27,
    .enablePin = 26,
};

static OSSMRefBoardV2Properties OSSMMotorProperties{
    .stepsPerMillimeter = STEP_PER_MM,
    .invertDirection = true,
    .enableActiveLow = true,
    .stepPin = 14,
    .directionPin = 27,
    .enablePin = 26,
    .alarmPin = 13,
    .inPositionPin = 4,
    .ADCPinCurrent = 36,
    .AmperePermV = 2.5e-3,
    .AmpereOffsetInmV = 1666,
    .currentThreshold = 0.05,
    .ADCPinVoltage = 39,
    .VoltPermV = 4.0e-2,
};

static iHSVServoV6Properties iHSVV6MotorProperties{
    .stepsPerMillimeter = STEP_PER_MM,
    .invertDirection = true,
    .enableActiveLow = true,
    .stepPin = 14,
    .directionPin = 27,
    .enablePin = 26,
    .alarmPin = 13,
    .inPositionPin = 4,
    .modbusRxPin = 16,
    .modbusTxPin = 17,
    .torqueThreshold = 100,
};

// VirtualMotor motor;
// GenericStepperMotor motor;
// OSSMRefBoardV2Motor motor;
iHSVServoV6Motor motor;

StrokeEngine Stroker;

// ESP32-SvelteKit #################################################################################
AsyncWebServer server(80);
ESP32SvelteKit esp32sveltekit(&server);

LightMqttSettingsService lightMqttSettingsService =
    LightMqttSettingsService(&server, esp32sveltekit.getFS(), esp32sveltekit.getSecurityManager());

LightStateService lightStateService = LightStateService(&server,
                                                        esp32sveltekit.getSecurityManager(),
                                                        esp32sveltekit.getMqttClient(),
                                                        &lightMqttSettingsService);

StrokeEngineControlService strokeEngineControlService = StrokeEngineControlService(&Stroker,
                                                                                   &server,
                                                                                   esp32sveltekit.getSecurityManager(),
                                                                                   esp32sveltekit.getMqttClient(),
                                                                                   &lightMqttSettingsService);

WebSocketRawDataStreamer PositionStream(&server);

/*#################################################################################################
##
##    C A L L B A C K S
##
##################################################################################################*/

void streamMotorData(unsigned int time, float position, float speed, float voltage, float current)
{
    // float voltage = 0.0;
    // float current = 0.0;
    // Serial.printf("Time: %d, Position: %f, Speed: %f, Voltage: %f, Current: %f\n", time, position, speed, voltage, current);
    PositionStream.streamRawData(time, position, speed, voltage, current);
}

/*#################################################################################################
##
##    T A S K S
##
##################################################################################################*/

// None

/*#################################################################################################
##
##    I S R ' S
##
##################################################################################################*/

// None

/*#################################################################################################
##
##    F U N C T I O N S
##
##################################################################################################*/

// None

/*#################################################################################################
##
##    M A I N   P R O G R A M
##
##################################################################################################*/

void setup()
{
    // start serial and filesystem
    Serial.begin(SERIAL_BAUD_RATE);

    // motor.begin(streamMotorData, 20);
    // motor.begin(&OSSMMotorProperties);
    // motor.begin(&genericMotorProperties);
    motor.begin(&iHSVV6MotorProperties);
    motor.attachPositionFeedback(streamMotorData, 500);
    // motor.setSensoredHoming(12, INPUT_PULLUP, true);

    motor.setMaxSpeed(MAX_SPEED);     // 2 m/s
    motor.setMaxAcceleration(100000); // 100 m/s^2
    // motor.setMachineGeometry(160.0, 5.0);

    Stroker.attachMotor(&motor);
    motor.enable();
    // motor.home();

    // Measure rail length (including homing)
    motor.measureRailLength(5.0);

    // start the framework and LUST-motion
    esp32sveltekit.setMDNSAppName("LUST-motion");
    esp32sveltekit.begin();
    MDNS.addService("stroking", "tcp", 80);
    MDNS.addServiceTxt("stroking", "tcp", "FirmwareVersion", FIRMWARE_VERSION);
    MDNS.addServiceTxt("stroking", "tcp", "DeviceID", SettingValue::format("LUST-motion-#{unique_id}"));

    // load the initial light settings
    lightStateService.begin();

    // start the light service
    lightMqttSettingsService.begin();

    strokeEngineControlService.begin();

    // start the server
    server.begin();

    // Wait a little bit
    // delay(1000);

    // Stroker.setParameter(StrokeParameter::SENSATION, 30.0, true);
    // Stroker.startPattern();
}

void loop()
{
    // Delete Arduino loop task, as it is not needed in this example
    vTaskDelete(NULL);
}
