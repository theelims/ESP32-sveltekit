/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <DownloadFirmwareService.h>

extern const uint8_t rootca_crt_bundle_start[] asm("_binary_src_certs_x509_crt_bundle_bin_start");
extern const uint8_t rootca_crt_bundle_end[] asm("_binary_src_certs_x509_crt_bundle_bin_end");

static EventSocket *_socket = nullptr;
static int previousProgress = 0;
static String *otaURL = nullptr;
JsonDocument doc;

void update_started()
{
    String output;
    doc["status"] = "preparing";
    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);
    ESP_LOGD(SVK_TAG, "HTTP onUpdate started");
}

void update_progress(int currentBytes, int totalBytes)
{
    doc["status"] = "progress";
    int progress = ((currentBytes * 100) / totalBytes);
    if (progress > previousProgress)
    {
        doc["progress"] = progress;
        JsonObject jsonObject = doc.as<JsonObject>();
        _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);
        ESP_LOGV(SVK_TAG, "HTTP update process at %d of %d bytes... (%d %%)", currentBytes, totalBytes, progress);
    }
    previousProgress = progress;
}

void updateTask(void *param)
{
    String url = *((String *)param);
    delete (String *)param; // Clean up the allocated memory

    WiFiClientSecure client;

#if ESP_ARDUINO_VERSION_MAJOR == 3
    client.setCACertBundle(rootca_crt_bundle_start, rootca_crt_bundle_end - rootca_crt_bundle_start);
#else
    client.setCACertBundle(rootca_crt_bundle_start);
#endif

    // client.setInsecure(); // For testing purposes only, remove this line for production code

    client.setTimeout(12000);

    httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    httpUpdate.rebootOnUpdate(true);

    String output;
    httpUpdate.onStart(update_started);
    httpUpdate.onProgress(update_progress);

    t_httpUpdate_return ret = httpUpdate.update(client, url.c_str());
    JsonObject jsonObject;

    // Reduce task priority to allow other tasks to run
    vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 1);

    bool _emitEvent = false;

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:

        doc["status"] = "error";
        doc["error"] = httpUpdate.getLastErrorString().c_str();
        _emitEvent = true;

        ESP_LOGE(SVK_TAG, "HTTP Update failed with error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
#ifdef SERIAL_INFO
        Serial.printf("HTTP Update failed with error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
#endif
        break;
    case HTTP_UPDATE_NO_UPDATES:

        doc["status"] = "error";
        doc["error"] = "Update failed, has same firmware version";
        _emitEvent = true;

        ESP_LOGE(SVK_TAG, "HTTP Update failed, has same firmware version");
#ifdef SERIAL_INFO
        Serial.println("HTTP Update failed, has same firmware version");
#endif
        break;
    case HTTP_UPDATE_OK:

        doc["status"] = "finished";
        _emitEvent = true;

        ESP_LOGI(SVK_TAG, "HTTP Update successful - Restarting");
#ifdef SERIAL_INFO
        Serial.println("HTTP Update successful - Restarting");
#endif
        break;
    }

    if (_emitEvent)
    {
        jsonObject = doc.as<JsonObject>();
        _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);
    }

    // delay to allow the event to be sent out
    vTaskDelay(100 / portTICK_PERIOD_MS);

    vTaskDelete(NULL);
}

DownloadFirmwareService::DownloadFirmwareService(PsychicHttpServer *server,
                                                 SecurityManager *securityManager,
                                                 EventSocket *socket) : _server(server),
                                                                        _securityManager(securityManager),
                                                                        _socket(socket)
{
}

void DownloadFirmwareService::begin()
{
    ::_socket = _socket;

    _socket->registerEvent(EVENT_DOWNLOAD_OTA);

    _server->on(GITHUB_FIRMWARE_PATH,
                HTTP_POST,
                _securityManager->wrapCallback(
                    std::bind(&DownloadFirmwareService::downloadUpdate, this, std::placeholders::_1, std::placeholders::_2),
                    AuthenticationPredicates::IS_ADMIN));

    ESP_LOGV(SVK_TAG, "Registered POST endpoint: %s", GITHUB_FIRMWARE_PATH);
}

esp_err_t DownloadFirmwareService::downloadUpdate(PsychicRequest *request, JsonVariant &json)
{
    if (!json.is<JsonObject>())
    {
        return request->reply(400);
    }

    String downloadURL = json["download_url"];
    ESP_LOGI(SVK_TAG, "Starting OTA from: %s", downloadURL.c_str());
#ifdef SERIAL_INFO
    Serial.println("Starting OTA from: " + downloadURL);
#endif

    doc["status"] = "preparing";
    doc["progress"] = 0;
    doc["error"] = "";

    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);

    // Allocate memory for the URL on the heap
    String *urlPtr = new String(downloadURL);

    if (xTaskCreatePinnedToCore(
            &updateTask,                // Function that should be called
            "Update",                   // Name of the task (for debugging)
            OTA_TASK_STACK_SIZE,        // Stack size (bytes)
            urlPtr,                     // Pass reference to this class instance
            (configMAX_PRIORITIES - 1), // Pretty high task priority
            NULL,                       // Task handle
            1                           // Have it on application core
            ) != pdPASS)
    {
        delete urlPtr; // Clean up if task creation fails
        ESP_LOGE(SVK_TAG, "Couldn't create download OTA task");
        return request->reply(500);
    }
    return request->reply(200);
}
