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

#include <HomeAssistant/HADiagnosticService.h>
#include <HomeAssistant/HAButton.h>
#include <HomeAssistant/HADevice.h>
#include <RestartService.h>

HADiagnosticService::HADiagnosticService(HAService *haService)
    : _haService(haService),
      _diagnosticSensors(haService, "diagnostics", _readDiagnosticState),
      _diagnosticSensorTimer(nullptr)
{
    _diagnosticSensors
        .addSensor("sensor", "status", "Status", "mdi:heart-pulse", HACategory::Diagnostic,
                   [](JsonObject &c)
                   {
                       c["value_template"] = "{{value_json.state}}";
                   })
        .addSensor("sensor", "free_heap", "Free Heap", "mdi:memory", HACategory::Diagnostic,
                   [](JsonObject &c)
                   {
                       c["value_template"]      = "{{value_json.free_heap_percent|round(1)}}";
                       c["unit_of_measurement"] = "%";
                       c["state_class"]         = "measurement";
                   })
        .addSensor("sensor", "core_temp", "Core Temperature", "mdi:thermometer", HACategory::Diagnostic,
                   [](JsonObject &c)
                   {
                       c["value_template"]      = "{{value_json.core_temp|round(1)}}";
                       c["unit_of_measurement"] = "°C";
                       c["device_class"]        = "temperature";
                       c["state_class"]         = "measurement";
                   });

    if (_haService != nullptr)
    {
        auto restart = std::unique_ptr<HAButton>(
            new HAButton(_haService, "restart",
                         []()
                         {
                             ESP_LOGI(TAG, "Restart button pressed via MQTT - restarting in 1 second");
                             vTaskDelay(pdMS_TO_TICKS(1000));
                             RestartService::restartNow();
                         },
                         [](JsonObject &c)
                         { c["payload_press"] = "PRESS"; }));
        restart->setName("Restart").setIcon("mdi:restart");
        _haService->mainDevice().registerControl(std::move(restart));
    }
}

void HADiagnosticService::begin()
{
    _diagnosticSensors.begin();

    _diagnosticSensorTimer = xTimerCreate(
        "HADiagSensor",
        pdMS_TO_TICKS(DIAGNOSTIC_SENSOR_UPDATE_INTERVAL_MS),
        pdTRUE, // Auto-reload
        this,
        _diagnosticSensorTimerCallback);

    if (_diagnosticSensorTimer != nullptr)
    {
        xTimerStart(_diagnosticSensorTimer, 0);
        ESP_LOGI(TAG, "Diagnostic sensor timer started (%d minute interval)",
                 DIAGNOSTIC_SENSOR_UPDATE_INTERVAL_MS / (60 * 1000));
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create diagnostic sensor timer");
    }
}

void HADiagnosticService::_readDiagnosticState(JsonObject &state)
{
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    float freeHeapPercent = (totalHeap > 0)
                                ? (static_cast<float>(freeHeap) * 100.0f / static_cast<float>(totalHeap))
                                : 0.0f;

    state["state"] = "online";
    state["free_heap_percent"] = freeHeapPercent;
    state["core_temp"] = temperatureRead();
}

void HADiagnosticService::_diagnosticSensorTimerCallback(TimerHandle_t timer)
{
    HADiagnosticService *service = static_cast<HADiagnosticService *>(pvTimerGetTimerID(timer));
    if (service != nullptr)
    {
        service->_diagnosticSensors.publishState();
    }
}
