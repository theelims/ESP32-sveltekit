#pragma once

/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

/**
 *   Board configuration for the OSSM Reference Board V1 & V2
 *   from Research and Desire
 *
 *   It supports the basic functions like stepper motor driver interface,
 *   Neopixel-LED, RS232 modbus communication, endstop and current/voltage sensor.
 *
 *   Depending on the version of the board, the following features are available:
 *
 *   V1:
 *   - stepper motor driver
 *   - Neopixel-LED
 *   - RS232 modbus communication
 *   - endstop switch
 *   - Motor drivers: Generic stepper and iHSV servo with firmware V6
 *
 *   V2:
 *   - stepper motor driver
 *   - Neopixel-LED
 *   - RS232 modbus communication
 *   - endstop switch
 *   - current/voltage sensor
 *   - Motor drivers: Generic stepper, iHSV servo with firmware V6
 *                    and OSSM Reference Board V2 with current sensor
 *
 **/

// Motor properties
#define STEP_PER_REV 2000       // 2000 steps per revolution
#define PULLEY_TEETH 20         // 20 teeth on the pulley
#define INVERT_DIRECTION true   // invert direction of the motor or S6
#define KEEP_OUT 5.0            // keep out distance in mm
#define SENSORLESS_TRIGGER 5.0  // trigger percentage for sensorless homing (Torque or Current)
#define BELT_PITCH 2            // belt pitch in mm
#define MAX_RPM 3000.0          // maximum RPM
#define MAX_ACCELERATION 100000 // maximum acceleration in mm/s^2
#define ENABLE_ACTIVE_LOW true  // enable pin is active low

// Pin definitions
#define STEP_PIN 14        // step pin
#define DIRECTION_PIN 27   // direction pin
#define ENABLE_PIN 26      // enable pin
#define ALARM_PIN 13       // alarm pin
#define IN_POSITION_PIN 4  // in position pin
#define ENDSTOP_PIN 12     // endstop pin
#define ADC_PIN_CURRENT 36 // current sensor ADC pin
#define ADC_PIN_VOLTAGE 39 // voltage sensor ADC pin
#define MODBUS_RX_PIN 16   // modbus rx pin
#define MODBUS_TX_PIN 17   // modbus tx pin
#define NEOPIXEL_PIN 25    // neopixel data pin

// Current sensor properties
#define AMPERE_PER_MILLIVOLT 2.5e-3     // ADC conversion factor current reading: 2.5mA per mV
#define AMPERE_OFFSET_IN_MILLIVOLT 1666 // ADC offset in mV for 0A
#define VOLT_PER_MILLIVOLT 4.0e-2       // ADC conversion factor voltage reading: 40mV per V
#define MAX_AMPERE 5.0                  // maximum range current sensor in Ampere
#define MAX_VOLTAGE 40.0                // maximum range voltage sensor in Volt

// Neopixel properties
#define NEOPIXEL_COUNT 1                           // number of neopixel LEDs
#define NEOPIXEL_METHODE NeoEsp32Rmt0Ws2812xMethod // NeoPixelBus method for neopixel
#define NEOPIXEL_COLOR_ORDER NeoGrbFeature         // NeoPixelBus color order for neopixel

// Available motor drivers - VIRTUAL driver is enabled by default
#define DRIVER_GENERIC_STEPPER   // Generic stepper driver with endstop switch
#define DRIVER_OSSM_REF_BOARD_V2 // OSSM Reference Board V2 with sensorless homing via current sensor
