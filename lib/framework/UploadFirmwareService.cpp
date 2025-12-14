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
#include <esp_app_format.h>
#include <strings.h>
#include <ArduinoJson.h>

using namespace std::placeholders; // for `_1` etc

UploadFirmwareService::UploadFirmwareService(PsychicHttpServer *server,
                                             SecurityManager *securityManager,
                                             EventSocket *socket) : _server(server),
                                                                    _securityManager(securityManager),
                                                                    _socket(socket)
{
    _md5[0] = '\0';  // Initialize MD5 buffer
}

void UploadFirmwareService::begin()
{
    _socket->registerEvent(EVENT_OTA_UPDATE);
    
    _server->maxUploadSize = MAX_FIRMWARE_SIZE;

    // Setup progress callback for Update library
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

    PsychicUploadHandler *uploadHandler = new PsychicUploadHandler();

    uploadHandler->onUpload(std::bind(&UploadFirmwareService::handleUpload, this, _1, _2, _3, _4, _5, _6));
    uploadHandler->onRequest(std::bind(&UploadFirmwareService::uploadComplete, this, _1));  // gets called after upload has been handled
    uploadHandler->onClose(std::bind(&UploadFirmwareService::handleEarlyDisconnect, this)); // gets called if client disconnects
    _server->on(UPLOAD_FIRMWARE_PATH, HTTP_POST, uploadHandler);

    ESP_LOGV(SVK_TAG, "Registered POST endpoint: %s", UPLOAD_FIRMWARE_PATH);
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

esp_err_t UploadFirmwareService::handleUpload(PsychicRequest *request,
                                              const String &filename,
                                              uint64_t index,
                                              uint8_t *data,
                                              size_t len,
                                              bool final)
{
    // quit if not authorized
    Authentication authentication = _securityManager->authenticateRequest(request);
    if (!AuthenticationPredicates::IS_ADMIN(authentication))
    {
        return handleError(request, 403, "Insufficient permissions to upload firmware");
    }

    if (index == 0) // Are we at the start of a new upload?
    {
        // check details of the file, to see if its a valid bin or md5 file
        std::string fname(filename.c_str());
        auto position = fname.find_last_of(".");
        
        // Check if extension exists to avoid undefined behavior
        if (position == std::string::npos)
        {
            return handleError(request, 406, "File has no extension");
        }
        
        std::string extension = fname.substr(position + 1);
        size_t fsize = request->contentLength();

        _fileType = ft_none;
        if (strcasecmp(extension.c_str(), "md5") == 0)  // Are we processing an MD5 file?
        {
            _fileType = ft_md5;
          
            if (len == MD5_LENGTH)
            {
                // Safe: _md5[MD5_LENGTH + 1] has space for 32 bytes + null terminator
                memcpy(_md5, data, MD5_LENGTH);
                _md5[MD5_LENGTH] = '\0';

                return ESP_OK;  // Finished processing MD5 file
            }
            else
            {
                _md5[0] = '\0';  // Clear any previously stored MD5 on invalid upload
                return handleError(request, 422, "MD5 must be exactly 32 bytes");
            }
        }
        else if (strcasecmp(extension.c_str(), "bin") == 0) // Are we processing a firmware binary?
        {
            _fileType = ft_firmware;
            
            ESP_LOGI(SVK_TAG, "Starting firmware upload: %s (%d bytes)", filename.c_str(), fsize);
#ifdef SERIAL_INFO
            Serial.printf("Starting firmware upload: %s (%d bytes)\n", filename.c_str(), fsize);
#endif
            
            // Validate firmware header (magic byte and chip type)
            if (!validateChipType(data, len))
            {
                return handleError(request, 503, "Wrong firmware for this device");
            }
            
            if (Update.begin(fsize - sizeof(esp_image_header_t)))
            {
                // Emit preparing status after validation succeeds
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
                    _md5[0] = '\0';  // clear md5 after setting it in Arduino Updater
                }
            }
            else
            {
                return handleError(request, 507, "Insufficient storage space");
            }
        }
        else // Are we processing an unsupported file type?
        {
            return handleError(request, 406, "File not a firmware binary or MD5 hash");
        }
    }
    else // we are continuing an existing upload
    {
        // Resumable upload: verify that Update was already started
        if (_fileType == ft_none || !Update.isRunning())
        {
            return handleError(request, 400, "Upload not initialized");
        }
    }

    // if we haven't dealt with an error so far, continue with the firmware update
    if (!request->_tempObject)
    {
        if (_fileType == ft_firmware)
        {
            if (Update.write(data, len) != len)
            {
                Update.abort();
                return handleError(request, 500, "Firmware write failed");
            }
            if (final)
            {
                if (!Update.end(true))
                {
                    // Get specific error message from Update library (includes MD5 mismatch)
                    String errorMsg = "Firmware update failed";
                    if (Update.hasError())
                    {
                        errorMsg = Update.errorString();
                    }
                    Update.abort();
                    return handleError(request, 500, errorMsg.c_str());
                }
            }
        }
    }

    return ESP_OK;
}

esp_err_t UploadFirmwareService::uploadComplete(PsychicRequest *request)
{
    // if we already handled an error in handleUpload, do nothing
    if (request->_tempObject)
    {
        return ESP_OK;
    }
    
    // if we completed uploading a md5 file create a JSON response
    if (_fileType == ft_md5)
    {
        if (strlen(_md5) == MD5_LENGTH)
        {
            PsychicJsonResponse response = PsychicJsonResponse(request, false);
            JsonObject root = response.getRoot();
            root["md5"] = _md5;
            return response.send();
        }
        return ESP_OK;
    }

    // if no error, send the success response
    if (_fileType == ft_firmware)
    {
        // Emit finished event
        if (_socket)
        {
            JsonDocument doc;
            doc["status"] = "finished";
            doc["progress"] = 100;
            JsonObject jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
            vTaskDelay(100 / portTICK_PERIOD_MS); // Give time for event to be sent
        }
        
        ESP_LOGI(SVK_TAG, "Firmware upload successful - Restarting");
#ifdef SERIAL_INFO
        Serial.println("Firmware upload successful - Restarting");
#endif
        
        // Reset progress tracker for next upload
        _previousProgress = 0;
        
        request->reply(200);
        RestartService::restartNow();
        return ESP_OK;
    }

    // if updated has an error send 500 response and log on Serial
    if (Update.hasError())
    {
        // Get specific error message from Update library
        String errorMsg = Update.errorString();
        if (errorMsg.length() == 0)
        {
            errorMsg = "Unknown update error";
        }
        
        // Reset progress tracker
        _previousProgress = 0;
        
        ESP_LOGE(SVK_TAG, "Update error: %s", errorMsg.c_str());
#ifdef SERIAL_INFO
        Update.printError(Serial);
#endif
        Update.abort();
        
        // handleError will emit the WebSocket event and send HTTP response
        return handleError(request, 500, errorMsg.c_str());
    }

    return ESP_OK;
}

esp_err_t UploadFirmwareService::handleError(PsychicRequest *request, int code, const char *message)
{
    // if we have had an error already, do nothing
    if (request->_tempObject)
    {
        return ESP_OK;
    }

    // Emit WebSocket error event for BIN files (skip for MD5 files)
    if (_fileType == ft_firmware && _socket && message)
    {
        JsonDocument doc;
        doc["status"] = "error";
        doc["error"] = message;
        JsonObject jsonObject = doc.as<JsonObject>();
        _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
    }
    
    // Log error
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

    // Reset state to allow new upload attempts
    _fileType = ft_none;
    _previousProgress = 0;
    
    // Abort any ongoing Update to clear error state
    Update.abort();
    
    // Mark this request as having encountered an error using _tempObject as a flag
    // (The pointer value itself is not used, only checked for NULL vs non-NULL)
    // The allocated memory is freed on request destruction (see PsychicRequest::~PsychicRequest())
    request->_tempObject = malloc(sizeof(int));
    return request->reply(code);
}

esp_err_t UploadFirmwareService::handleEarlyDisconnect()
{
    // if updated has not ended on connection close, abort it
    if (!Update.end(true))
    {
        ESP_LOGE(SVK_TAG, "Update error on early disconnect:");
#ifdef SERIAL_INFO
        Update.printError(Serial);
#endif
        Update.abort();
        return ESP_OK;
    }
    return ESP_OK;
}
