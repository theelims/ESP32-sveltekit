#pragma once

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <OTAUpdateCallback.h>
#include <EventSocket.h>
#include <FirmwareUpdateEvents.h>
#include <ArduinoJson.h>

/**
 * @brief OTA update callback that reports progress via EventSocket (WebSocket)
 *
 * Implements OTAUpdateCallback to send firmware update progress to the
 * Web UI frontend via EventSocket events. Used by DownloadFirmwareService
 * for Web UI-initiated firmware updates.
 */
class WebSocketOTACallback : public OTAUpdateCallback
{
public:
    explicit WebSocketOTACallback(EventSocket *socket) : _socket(socket) {}

    void onUpdateStart() override
    {
        if (_socket)
        {
            JsonDocument doc;
            doc["status"] = "preparing";
            doc["progress"] = 0;
            doc["bytes_written"] = 0;
            doc["total_bytes"] = 0;
            JsonObject jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
        }
    }

    void onUpdateProgress(int currentBytes, int totalBytes) override
    {
        if (_socket)
        {
            int progress = totalBytes > 0 ? ((currentBytes * 100) / totalBytes) : 0;
            JsonDocument doc;
            doc["status"] = "progress";
            doc["progress"] = progress;
            doc["bytes_written"] = currentBytes;
            doc["total_bytes"] = totalBytes;
            JsonObject jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
        }
    }

    void onUpdateFinish() override
    {
        if (_socket)
        {
            JsonDocument doc;
            doc["status"] = "finished";
            doc["progress"] = 100;
            JsonObject jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
        }
    }

    void onUpdateError(const String &errorMessage) override
    {
        if (_socket)
        {
            JsonDocument doc;
            doc["status"] = "error";
            doc["error"] = errorMessage;
            JsonObject jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
        }
    }

private:
    EventSocket *_socket;
};
