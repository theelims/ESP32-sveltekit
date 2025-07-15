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
    AnalyticsService(EventSocket *socket) : _socket(socket) {};

    void begin()
    {
        _socket->registerEvent(EVENT_ANALYTICS);
    }

    void loop()
    {
        if (millis() - lastMillis > ANALYTICS_INTERVAL)
        {
            lastMillis = millis();
            JsonDocument doc;
            doc["uptime"] = millis() / 1000;
            doc["free_heap"] = ESP.getFreeHeap();
            doc["used_heap"] = ESP.getHeapSize() - ESP.getFreeHeap();
            doc["total_heap"] = ESP.getHeapSize();
            doc["min_free_heap"] = ESP.getMinFreeHeap();
            doc["max_alloc_heap"] = ESP.getMaxAllocHeap();
            doc["fs_used"] = ESPFS.usedBytes();
            doc["fs_total"] = ESPFS.totalBytes();
            doc["core_temp"] = temperatureRead();
            if (psramFound()) {
                doc["free_psram"] = ESP.getFreePsram();
                doc["used_psram"] = ESP.getPsramSize() - ESP.getFreePsram();
                doc["psram_size"] = ESP.getPsramSize();
            }

            JsonObject jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_ANALYTICS, jsonObject);
        }
    };

protected:
    EventSocket *_socket;

    unsigned long lastMillis = 0;
};
