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

#include <OTAUpdateCallback.h>
#include <EventSocket.h>
#include <FirmwareUpdateEvents.h>
#include <ArduinoJson.h>

// Forward declaration — avoids circular include
class HAUpdateService;

/**
 * @brief OTA update callback for dual-channel progress reporting
 *
 * Reports OTA progress to:
 * - WebSocket (Web UI) — directly via EventSocket
 * - MQTT (Home Assistant) — delegated to HAUpdateService
 *
 * Allocated on the heap by HAUpdateService, passed to DownloadFirmwareService::startOTAUpdate(),
 * and deleted by the OTA task after completion.
 */
class HAOTACallback : public OTAUpdateCallback
{
public:
    /**
     * @brief Construct a dual-channel OTA callback
     * @param haUpdateService HAUpdateService for MQTT state publishing
     * @param eventSocket EventSocket for Web UI progress (can be nullptr)
     * @param onErrorCleanup Optional callback invoked on error (e.g., to release OTA-in-progress flag)
     */
    HAOTACallback(HAUpdateService *haUpdateService,
                  EventSocket *eventSocket,
                  std::function<void()> onErrorCleanup = nullptr)
        : _haUpdateService(haUpdateService),
          _eventSocket(eventSocket),
          _onErrorCleanup(onErrorCleanup),
          _lastMqttProgress(-1)
    {
    }

    void onUpdateStart() override
    {
        ESP_LOGI("HAOTACallback", "OTA Update started - reporting to MQTT and WebSocket");
        _reportMqttProgress(0);
        _emitWebSocketEvent("preparing", 0, 0, 0);
    }

    void onUpdateProgress(int currentBytes, int totalBytes) override
    {
        int progress = totalBytes > 0 ? ((currentBytes * 100) / totalBytes) : 0;

        // MQTT: report at 1% and every 5% to reduce traffic
        if ((progress == 1 || progress % 5 == 0 || progress == 100) && progress != _lastMqttProgress)
        {
            _reportMqttProgress(progress);
            _lastMqttProgress = progress;
        }

        // WebSocket: report every change
        _emitWebSocketEvent("progress", progress, currentBytes, totalBytes);
    }

    void onUpdateFinish() override
    {
        ESP_LOGI("HAOTACallback", "OTA Update finished - publishing 100%% to both channels");
        _reportMqttProgress(100);
        _emitWebSocketEvent("finished", 100, 0, 0);
    }

    void onUpdateError(const String &errorMessage) override
    {
        ESP_LOGE("HAOTACallback", "OTA Update failed: %s", errorMessage.c_str());

        // Report error to WebSocket
        if (_eventSocket != nullptr)
        {
            JsonDocument doc;
            doc["status"] = "error";
            doc["error"] = errorMessage;
            JsonObject jsonObject = doc.as<JsonObject>();
            _eventSocket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
        }

        // Delegate MQTT idle-state publish to HAUpdateService
        if (_haUpdateService != nullptr)
        {
            _haUpdateService->publishOTAError();
        }

        // Invoke error cleanup callback (e.g., release OTA flag)
        if (_onErrorCleanup)
        {
            _onErrorCleanup();
        }
    }

private:
    HAUpdateService *_haUpdateService;
    EventSocket *_eventSocket;
    std::function<void()> _onErrorCleanup;
    int _lastMqttProgress;

    void _reportMqttProgress(int progress)
    {
        if (_haUpdateService != nullptr)
        {
            _haUpdateService->publishOTAProgress(progress);
        }
    }

    void _emitWebSocketEvent(const char *status, int progress, int bytesWritten, int totalBytes)
    {
        if (_eventSocket == nullptr)
            return;

        JsonDocument doc;
        doc["status"] = status;
        doc["progress"] = progress;

        if (bytesWritten > 0 || totalBytes > 0)
        {
            doc["bytes_written"] = bytesWritten;
            doc["total_bytes"] = totalBytes;
        }

        JsonObject jsonObject = doc.as<JsonObject>();
        _eventSocket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
    }
};
