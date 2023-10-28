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

#define VIRTUAL

#include <ESP32SvelteKit.h>
#include <ESPmDNS.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include <StrokeEngine.h>
#include <SettingValue.h>
#include <StrokeEngineControlService.h>
#include <WebSocketRawDataStreaming.h>
#ifdef VIRTUAL
#include <motor/virtualMotor.h>
#else
#include <motor/genericStepper.h>
#endif

/*#################################################################################################
##
##    G L O B A L    D E F I N I T I O N S   &   D E C L A R A T I O N S
##
##################################################################################################*/

// StrokeEngine ###################################################################################
// Calculation Aid:
#define STEP_PER_REV 2000 // How many steps per revolution of the motor (S1 off, S2 on, S3 on, S4 off)
#define PULLEY_TEETH 20   // How many teeth has the pulley
#define BELT_PITCH 2      // What is the timing belt pitch in mm
#define MAX_RPM 3000.0    // Maximum RPM of motor
#define STEP_PER_MM STEP_PER_REV / (PULLEY_TEETH * BELT_PITCH)
#define MAX_SPEED (MAX_RPM / 60.0) * PULLEY_TEETH *BELT_PITCH

#ifndef VIRTUAL
static motorProperties servoMotor{
    .stepsPerMillimeter = STEP_PER_MM,
    .invertDirection = true,
    .enableActiveLow = true,
    .stepPin = OSSM[0].pin.servoPul,
    .directionPin = OSSM[0].pin.servoDir,
    .enablePin = OSSM[0].pin.servoEna};
#endif

#ifdef VIRTUAL
VirtualMotor motor;
#else
GenericStepperMotor motor;
#endif

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

void printSpeedPositionOnSerial(unsigned int time, float position, float speed)
{
    PositionStream.streamRawData(time, position, speed);
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

#ifdef VIRTUAL
    motor.begin(printSpeedPositionOnSerial, 20);
#else
    motor.begin();
    motor.attachPositionFeedback(printSpeedPositionOnSerial, 20);
    motor.setSensoredHoming(OSSM[0].pin.lmt1, INPUT_PULLUP, true);
#endif
    motor.setMaxSpeed(MAX_SPEED);     // 2 m/s
    motor.setMaxAcceleration(100000); // 100 m/s^2
    motor.setMachineGeometry(160.0, 5.0);

    Stroker.attachMotor(&motor);
    motor.enable();
    motor.home();

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

    // motor.home(homingNotification);

    // Send available patterns as JSON
    // mqttPublish("/config", getPatternJSON());

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
