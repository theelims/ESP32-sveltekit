#pragma once

/**
 *   LUST-motion
 *
 *  Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

// Motor properties
#define STEP_PER_REV 2000
#define PULLEY_TEETH 20
#define INVERT_DIRECTION true
#define KEEP_OUT 5.0
#define SENSORLESS_TRIGGER 5.0
#define BELT_PITCH 2
#define MAX_RPM 3000.0 // Maximum RPM of motor
#define MAX_ACCELERATION 100000
#define ENABLE_ACTIVE_LOW true

// Pin definitions
#define STEP_PIN 14
#define DIRECTION_PIN 27
#define ENABLE_PIN 26
#define ALARM_PIN 13
#define IN_POSITION_PIN 4
#define ENDSTOP_PIN 12
#define ADC_PIN_CURRENT 36
#define ADC_PIN_VOLTAGE 39
#define MODBUS_RX_PIN 16
#define MODBUS_TX_PIN 17
#define NEOPIXEL_PIN 25

// Board properties
#define AMPERE_PER_MILLIVOLT 2.5e-3
#define AMPERE_OFFSET_IN_MILLIVOLT 1666
#define VOLT_PER_MILLIVOLT 4.0e-2
#define MAX_AMPERE 5.0
#define NEOPIXEL_COUNT 1
#define NEOPIXEL_METHODE NeoEsp32Rmt0Ws2812xMethod
#define NEOPIXEL_COLOR_ORDER NeoGrbFeature

// Available motor drivers
// VIRTUAL driver is enabled by default
#define DRIVER_GENERIC_STEPPER
#define DRIVER_OSSM_REF_BOARD_V2
#define DREIVER_IHSV_SERVO_V6
