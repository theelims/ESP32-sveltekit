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

#include <UploadFirmwareService.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_app_format.h>
#include <strings.h>
#include <ArduinoJson.h>

using namespace std::placeholders; // for `_1` etc

UploadFirmwareService::UploadFirmwareService(AsyncWebServer *server,
                                             SecurityManager *securityManager,
                                             EventSocket *socket) : _server(server),
                                                                    _securityManager(securityManager),
                                                                    _socket(socket)
{
    _md5[0] = '\0';
}

void UploadFirmwareService::begin()
{
    if (!_socket->isEventValid(EVENT_OTA_UPDATE))
    {
        _socket->registerEvent(EVENT_OTA_UPDATE);
    }

    _maxFirmwareSize = getMaxFirmwareSize();

    Update.onProgress([this](size_t progress, size_t total) {
        if (_socket && total > 0) {
            int percentComplete = (progress * 100) / total;
            if (percentComplete > _previousProgress || progress == total) {
                JsonDocument doc;
                doc["status"] = "progress";
                doc["progress"] = percentComplete;
                doc["bytes_written"] = progress;
                doc["total_bytes"] = total;

                JsonObject jsonObject = doc.as<JsonObject>();
                _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);

                ESP_LOGV(SVK_TAG, "Firmware upload process at %d of %d bytes... (%d %%)", progress, total, percentComplete);

                _previousProgress = percentComplete;
            }
        }
    });

    _server->on(UPLOAD_FIRMWARE_PATH, HTTP_POST,
        [this](AsyncWebServerRequest *request) { uploadComplete(request); },
        [this](AsyncWebServerRequest *request, String filename, size_t index,
               uint8_t *data, size_t len, bool final) {
            handleUpload(request, filename, index, data, len, final);
        });

    ESP_LOGV(SVK_TAG, "Registered POST endpoint: %s", UPLOAD_FIRMWARE_PATH);
}

size_t UploadFirmwareService::getMaxFirmwareSize()
{
    const esp_partition_t* update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition != NULL) {
        ESP_LOGI(SVK_TAG, "Max firmware size: %d bytes (from OTA partition)", update_partition->size);
        return update_partition->size;
    }
    
    // Fallback if partition query fails (should never happen)
    ESP_LOGW(SVK_TAG, "Could not determine OTA partition size, using fallback of 2MB");
    return 2097152; // 2 MB fallback
}

bool UploadFirmwareService::validateChipType(uint8_t *data, size_t len)
{
    if (len <= 12)
    {
        return false; // Not enough data to validate - firmware is invalid
    }
    
    // Check magic byte at offset 0
    if (data[0] != ESP_MAGIC_BYTE)
    {
        return false;
    }
    
    // Check chip ID at offset 12
    if (data[12] != ESP_CHIP_ID)
    {
        return false;
    }
    
    return true;
}

void UploadFirmwareService::handleUpload(AsyncWebServerRequest *request,
                                         const String &filename,
                                         size_t index,
                                         uint8_t *data,
                                         size_t len,
                                         bool final)
{
    Authentication authentication = _securityManager->authenticateRequest(request);
    if (!AuthenticationPredicates::IS_ADMIN(authentication))
    {
        handleError(request, 403, "Insufficient permissions to upload firmware");
        return;
    }

    if (index == 0)
    {
        std::string fname(filename.c_str());
        auto position = fname.find_last_of(".");

        if (position == std::string::npos)
        {
            handleError(request, 406, "File has no extension");
            return;
        }

        std::string extension = fname.substr(position + 1);
        size_t fsize = request->contentLength();

        _fileType = ft_none;
        if (strcasecmp(extension.c_str(), "md5") == 0)
        {
            _fileType = ft_md5;

            if (len == MD5_LENGTH)
            {
                memcpy(_md5, data, MD5_LENGTH);
                _md5[MD5_LENGTH] = '\0';
                return;
            }
            else
            {
                _md5[0] = '\0';
                handleError(request, 422, "MD5 must be exactly 32 bytes");
                return;
            }
        }
        else if (strcasecmp(extension.c_str(), "bin") == 0)
        {
            _fileType = ft_firmware;

            if (fsize > _maxFirmwareSize)
            {
                char errorMsg[64];
                snprintf(errorMsg, sizeof(errorMsg),
                        "Firmware too large: %.2f MB (max: %.2f MB)",
                        fsize / 1024.0 / 1024.0,
                        _maxFirmwareSize / 1024.0 / 1024.0);
                handleError(request, 413, errorMsg);
                return;
            }

            ESP_LOGI(SVK_TAG, "Starting firmware upload: %s (%d bytes)", filename.c_str(), fsize);
#ifdef SERIAL_INFO
            Serial.printf("Starting firmware upload: %s (%d bytes)\n", filename.c_str(), fsize);
#endif

            if (!validateChipType(data, len))
            {
                handleError(request, 503, "Wrong firmware for this device");
                return;
            }

            if (Update.begin(fsize))
            {
                request->onDisconnect([this]() { handleEarlyDisconnect(); });

                if (_socket)
                {
                    JsonDocument doc;
                    doc["status"] = "preparing";
                    doc["progress"] = 0;
                    JsonObject jsonObject = doc.as<JsonObject>();
                    _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
                }

                if (strlen(_md5) == MD5_LENGTH)
                {
                    Update.setMD5(_md5);
                    ESP_LOGI(SVK_TAG, "MD5 hash for validation: %s", _md5);
#ifdef SERIAL_INFO
                    Serial.printf("MD5 hash for validation: %s\n", _md5);
#endif
                    _md5[0] = '\0';
                }
            }
            else
            {
                handleError(request, 507, "Insufficient storage space");
                return;
            }
        }
        else
        {
            handleError(request, 406, "File not a firmware binary or MD5 hash");
            return;
        }
    }
    else
    {
        if (_fileType == ft_none || !Update.isRunning())
        {
            handleError(request, 400, "Upload not initialized");
            return;
        }
    }

    if (!request->_tempObject)
    {
        if (_fileType == ft_firmware)
        {
            if (Update.write(data, len) != len)
            {
                Update.abort();
                handleError(request, 500, "Firmware write failed");
                return;
            }
            if (final)
            {
                if (!Update.end(true))
                {
                    String errorMsg = "Firmware update failed";
                    if (Update.hasError())
                    {
                        errorMsg = Update.errorString();
                    }
                    Update.abort();
                    handleError(request, 500, errorMsg.c_str());
                    return;
                }
            }
        }
    }
}

void UploadFirmwareService::uploadComplete(AsyncWebServerRequest *request)
{
    if (request->_tempObject)
    {
        return;
    }

    if (_fileType == ft_md5)
    {
        if (strlen(_md5) == MD5_LENGTH)
        {
            AsyncJsonResponse *response = new AsyncJsonResponse();
            JsonObject root = response->getRoot();
            root["md5"] = _md5;
            response->setLength();
            request->send(response);
            return;
        }
        request->send(200);
        return;
    }

    if (_fileType == ft_firmware)
    {
        if (_socket)
        {
            JsonDocument doc;
            doc["status"] = "finished";
            doc["progress"] = 100;
            JsonObject jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        ESP_LOGI(SVK_TAG, "Firmware upload successful - Restarting");
#ifdef SERIAL_INFO
        Serial.println("Firmware upload successful - Restarting");
#endif

        _previousProgress = 0;

        request->send(200);
        RestartService::restartNow();
        return;
    }

    if (Update.hasError())
    {
        String errorMsg = Update.errorString();
        if (errorMsg.length() == 0)
        {
            errorMsg = "Unknown update error";
        }

        _previousProgress = 0;

        ESP_LOGE(SVK_TAG, "Update error: %s", errorMsg.c_str());
#ifdef SERIAL_INFO
        Update.printError(Serial);
#endif
        Update.abort();

        handleError(request, 500, errorMsg.c_str());
        return;
    }

    request->send(200);
}

void UploadFirmwareService::handleError(AsyncWebServerRequest *request, int code, const char *message)
{
    if (request->_tempObject)
    {
        return;
    }

    if (_fileType == ft_firmware && _socket && message)
    {
        JsonDocument doc;
        doc["status"] = "error";
        doc["error"] = message;
        JsonObject jsonObject = doc.as<JsonObject>();
        _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
    }

    if (message)
    {
        ESP_LOGE(SVK_TAG, "Firmware upload failed (%d): %s", code, message);
#ifdef SERIAL_INFO
        Serial.printf("Firmware upload failed (%d): %s\n", code, message);
#endif
    }
    else
    {
        ESP_LOGE(SVK_TAG, "Firmware upload failed with error code: %d", code);
#ifdef SERIAL_INFO
        Serial.printf("Firmware upload failed with error code: %d\n", code);
#endif
    }

    _fileType = ft_none;
    _previousProgress = 0;

    Update.abort();

    // _tempObject freed by AsyncWebServerRequest destructor
    request->_tempObject = malloc(sizeof(int));
    request->send(code);
}

void UploadFirmwareService::handleEarlyDisconnect()
{
    if (!Update.isRunning())
    {
        return;
    }
    if (!Update.end(true))
    {
        ESP_LOGE(SVK_TAG, "Update error on early disconnect:");
#ifdef SERIAL_INFO
        Update.printError(Serial);
#endif
        Update.abort();
    }
}
