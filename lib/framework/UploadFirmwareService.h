#ifndef UploadFirmwareService_h
#define UploadFirmwareService_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *   Copyright (C) 2025 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Arduino.h>

#include <Update.h>
#include <WiFi.h>

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <SecurityManager.h>
#include <RestartService.h>
#include <EventSocket.h>
#include <FirmwareUpdateEvents.h>

#define UPLOAD_FIRMWARE_PATH "/rest/uploadFirmware"

// Firmware upload constants
constexpr size_t MD5_LENGTH = 32;              // MD5 hash length
constexpr uint8_t ESP_MAGIC_BYTE = 0xE9;       // ESP binary magic byte

// ESP32 chip type identifiers (byte offset 12 in firmware)
#if CONFIG_IDF_TARGET_ESP32
    constexpr uint8_t ESP_CHIP_ID = 0;
#elif CONFIG_IDF_TARGET_ESP32S2
    constexpr uint8_t ESP_CHIP_ID = 2;
#elif CONFIG_IDF_TARGET_ESP32C3
    constexpr uint8_t ESP_CHIP_ID = 5;
#elif CONFIG_IDF_TARGET_ESP32S3
    constexpr uint8_t ESP_CHIP_ID = 9;
#else
    #error "Unsupported ESP32 target"
#endif

enum FileType
{
    ft_none = 0,
    ft_firmware = 1,
    ft_md5 = 2
};

/**
 * @brief Service for handling firmware uploads over HTTP with OTA support
 * 
 * Supports chunked uploads of .bin firmware files and .md5 hash files for validation.
 * Emits real-time progress updates via WebSocket and validates chip compatibility.
 */
class UploadFirmwareService
{
public:
    UploadFirmwareService(AsyncWebServer *server, SecurityManager *securityManager, EventSocket *socket);

    void begin();

private:
    AsyncWebServer *_server;
    SecurityManager *_securityManager;
    EventSocket *_socket;

    char _md5[MD5_LENGTH + 1];
    FileType _fileType = ft_none;
    int _previousProgress = 0;
    size_t _maxFirmwareSize = 0;

    size_t getMaxFirmwareSize();
    bool validateChipType(uint8_t *data, size_t len);

    void handleUpload(AsyncWebServerRequest *request,
                      const String &filename,
                      size_t index,
                      uint8_t *data,
                      size_t len,
                      bool final);

    void uploadComplete(AsyncWebServerRequest *request);
    void handleError(AsyncWebServerRequest *request, int code, const char *message = nullptr);
    void handleEarlyDisconnect();
};

#endif // end UploadFirmwareService_h
