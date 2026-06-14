#pragma once

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2025 theelims
 *   Copyright (C) 2026 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Arduino.h>
#include <HomeAssistant/HAService.h>
#include <HomeAssistant/HAGroupedSensorPublisher.h>

/**
 * @brief Home Assistant diagnostic entities service
 *
 * Provides standard diagnostic entities for any ESP32 device:
 * - Status sensor (device registration, "online")
 * - Free heap memory sensor (percentage)
 * - Core temperature sensor (°C)
 * - Restart button
 *
 * The three sensors share one MQTT state topic via HAGroupedSensorPublisher.
 * The restart button is registered as an HAButton on the main HADevice.
 * Sensor values are refreshed periodically via a FreeRTOS timer.
 */
class HADiagnosticService
{
public:
    static constexpr const char *TAG = "HADiagnosticSvc";

    /// Diagnostic sensor update interval (1 minute)
    static constexpr uint32_t DIAGNOSTIC_SENSOR_UPDATE_INTERVAL_MS = 1 * 60 * 1000;

    HADiagnosticService(HAService *haService);

    void begin();

private:
    HAService *_haService;
    HAGroupedSensorPublisher _diagnosticSensors;
    TimerHandle_t _diagnosticSensorTimer;

    static void _readDiagnosticState(JsonObject &state);
    static void _diagnosticSensorTimerCallback(TimerHandle_t timer);
};
