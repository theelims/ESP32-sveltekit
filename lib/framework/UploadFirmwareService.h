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

#include <PsychicHttp.h>
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
    /**
     * @brief Construct firmware upload service
     * @param server PsychicHttpServer instance for handling HTTP requests
     * @param securityManager Security manager for authentication
     * @param socket EventSocket for emitting real-time progress updates
     */
    UploadFirmwareService(PsychicHttpServer *server, SecurityManager *securityManager, EventSocket *socket);

    /**
     * @brief Initialize the service and register HTTP endpoints
     * Sets up upload handlers and progress callbacks
     */
    void begin();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    EventSocket *_socket;

    // Upload state (per-instance to support concurrent uploads)
    char _md5[MD5_LENGTH + 1];
    FileType _fileType = ft_none;
    int _previousProgress = 0;
    size_t _maxFirmwareSize = 0;

    /**
     * @brief Get maximum firmware size from OTA partition
     * @return Size of OTA partition in bytes, or 2MB fallback if not available
     */
    size_t getMaxFirmwareSize();

    /**
     * @brief Validate firmware chip type matches target device
     * @param data First chunk of firmware data
     * @param len Length of data chunk
     * @return true if magic byte and chip ID are valid, false otherwise
     */
    bool validateChipType(uint8_t *data, size_t len);

    /**
     * @brief Handle incoming firmware upload chunks. Called once per chunk.
     * @param request HTTP request object
     * @param filename Uploaded file name
     * @param index Byte offset of this chunk (0 for first chunk)
     * @param data Chunk data buffer
     * @param len Chunk length in bytes
     * @param final true if this is the last chunk
     * @return ESP_OK on success, error code on failure
     */
    esp_err_t handleUpload(PsychicRequest *request,
                           const String &filename,
                           uint64_t index,
                           uint8_t *data,
                           size_t len,
                           bool final);
    
    /**
     * @brief Called after upload finished (i.e. all chunks received)
     * @param request HTTP request object
     * @return ESP_OK on success, error code on failure
     */
    esp_err_t uploadComplete(PsychicRequest *request);
    
    /**
     * @brief Handle upload errors and emit error events
     * @param request HTTP request object
     * @param code HTTP status code to return
     * @param message Optional error message (default: nullptr)
     * @return ESP_OK (error already handled)
     */
    esp_err_t handleError(PsychicRequest *request, int code, const char *message = nullptr);
    
    /**
     * @brief Handle client disconnection during upload
     * @return ESP_OK on successful cleanup
     */
    esp_err_t handleEarlyDisconnect();
};

#endif // end UploadFirmwareService_h
