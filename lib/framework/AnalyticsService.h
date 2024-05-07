#pragma once

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESPFS.h>
#include <EventSocket.h>

#define MAX_ESP_ANALYTICS_SIZE 1024
#define EVENT_ANALYTICS "analytics"
#define ANALYTICS_INTERVAL 2000

class AnalyticsService
{
public:
    AnalyticsService(EventSocket *socket) : _socket(socket){};

    void begin()
    {
        _socket->registerEvent(EVENT_ANALYTICS);

        xTaskCreatePinnedToCore(
            this->_loopImpl,            // Function that should be called
            "Analytics Service",        // Name of the task (for debugging)
            5120,                       // Stack size (bytes)
            this,                       // Pass reference to this class instance
            (tskIDLE_PRIORITY),         // task priority
            NULL,                       // Task handle
            ESP32SVELTEKIT_RUNNING_CORE // Pin to application core
        );
    };

protected:
    EventSocket *_socket;

    static void _loopImpl(void *_this) { static_cast<AnalyticsService *>(_this)->_loop(); }
    void _loop()
    {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        JsonDocument doc;
        while (1)
        {
            doc["uptime"] = millis() / 1000;
            doc["free_heap"] = ESP.getFreeHeap();
            doc["total_heap"] = ESP.getHeapSize();
            doc["min_free_heap"] = ESP.getMinFreeHeap();
            doc["max_alloc_heap"] = ESP.getMaxAllocHeap();
            doc["fs_used"] = ESPFS.usedBytes();
            doc["fs_total"] = ESPFS.totalBytes();
            doc["core_temp"] = temperatureRead();

            JsonObject jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_ANALYTICS, jsonObject);

            vTaskDelayUntil(&xLastWakeTime, ANALYTICS_INTERVAL / portTICK_PERIOD_MS);
        }
    };
};
