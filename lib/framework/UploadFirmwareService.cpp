/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <UploadFirmwareService.h>
#include <esp_ota_ops.h>
#include <esp_app_format.h>

using namespace std::placeholders; // for `_1` etc

static char md5[33] = "\0";

static FileType fileType = ft_none;

UploadFirmwareService::UploadFirmwareService(PsychicHttpServer *server,
                                             SecurityManager *securityManager) : _server(server),
                                                                                 _securityManager(securityManager)
{
}

void UploadFirmwareService::begin()
{
    _server->maxUploadSize = 2300000; // 2.3 MB

    PsychicUploadHandler *uploadHandler = new PsychicUploadHandler();

    uploadHandler->onUpload(std::bind(&UploadFirmwareService::handleUpload, this, _1, _2, _3, _4, _5, _6));
    uploadHandler->onRequest(std::bind(&UploadFirmwareService::uploadComplete, this, _1));  // gets called after upload has been handled
    uploadHandler->onClose(std::bind(&UploadFirmwareService::handleEarlyDisconnect, this)); // gets called if client disconnects
    _server->on(UPLOAD_FIRMWARE_PATH, HTTP_POST, uploadHandler);

    ESP_LOGV("UploadFirmwareService", "Registered POST endpoint: %s", UPLOAD_FIRMWARE_PATH);
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
        return handleError(request, 403); // forbidden
    }

    // at init
    if (!index)
    {
        // check details of the file, to see if its a valid bin or json file
        std::string fname(filename.c_str());
        auto position = fname.find_last_of(".");
        std::string extension = fname.substr(position + 1);
        size_t fsize = request->contentLength();

        fileType = ft_none;
        if ((extension == "bin") && (fsize > 1000000))
        {
            fileType = ft_firmware;
        }
        else if (extension == "md5")
        {
            fileType = ft_md5;
            if (len == 32)
            {
                memcpy(md5, data, 32);
                md5[32] = '\0';
            }
            return ESP_OK;
        }
        else
        {
            md5[0] = '\0';
            return handleError(request, 406); // Not Acceptable - unsupported file type
        }

        if (fileType == ft_firmware)
        {
            // Check firmware header, 0xE9 magic offset 0 indicates esp bin, chip offset 12: esp32:0, S2:2, C3:5
#if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
            if (len > 12 && (data[0] != 0xE9 || data[12] != 0))
            {
                return handleError(request, 503); // service unavailable
            }
#elif CONFIG_IDF_TARGET_ESP32S2
            if (len > 12 && (data[0] != 0xE9 || data[12] != 2))
            {
                return handleError(request, 503); // service unavailable
            }
#elif CONFIG_IDF_TARGET_ESP32C3
            if (len > 12 && (data[0] != 0xE9 || data[12] != 5))
            {
                return handleError(request, 503); // service unavailable
            }
#elif CONFIG_IDF_TARGET_ESP32S3
            if (len > 12 && (data[0] != 0xE9 || data[12] != 9))
            {
                return handleError(request, 503); // service unavailable
            }
#endif
            // it's firmware - initialize the ArduinoOTA updater
            if (Update.begin(fsize - sizeof(esp_image_header_t)))
            {
                if (strlen(md5) == 32)
                {
                    Update.setMD5(md5);
                    md5[0] = '\0';
                }
            }
            else
            {
                return handleError(request, 507); // failed to begin, send an error response Insufficient Storage
            }
        }
    }

    // if we haven't delt with an error, continue with the firmware update
    if (!request->_tempObject)
    {
        if (Update.write(data, len) != len)
        {
            handleError(request, 500);
        }
        if (final)
        {
            if (!Update.end(true))
            {
                handleError(request, 500);
            }
        }
    }

    return ESP_OK;
}

esp_err_t UploadFirmwareService::uploadComplete(PsychicRequest *request)
{
    // if we completed uploading a md5 file create a JSON response
    if (fileType == ft_md5)
    {
        if (strlen(md5) == 32)
        {
            PsychicJsonResponse response = PsychicJsonResponse(request, false);
            JsonObject root = response.getRoot();
            root["md5"] = md5;
            return response.send();
        }
        return ESP_OK;
    }

    // if no error, send the success response
    if (!request->_tempObject)
    {
        request->reply(200);
        RestartService::restartNow();
        return ESP_OK;
    }

    // if updated has an error send 500 response and log on Serial
    if (Update.hasError())
    {
        Update.printError(Serial);
        Update.abort();
        handleError(request, 500);
    }

    return ESP_OK;
}

esp_err_t UploadFirmwareService::handleError(PsychicRequest *request, int code)
{
    // if we have had an error already, do nothing
    if (request->_tempObject)
    {
        return ESP_OK;
    }

    // send the error code to the client and record the error code in the temp object
    request->_tempObject = new int(code);
    return request->reply(code);
}

esp_err_t UploadFirmwareService::handleEarlyDisconnect()
{
    // if updated has not ended on connection close, abort it
    if (!Update.end(true))
    {
        Update.printError(Serial);
        Update.abort();
        return ESP_OK;
    }
    return ESP_OK;
}
