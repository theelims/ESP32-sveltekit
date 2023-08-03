/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <DownloadFirmwareService.h>
static const char *TAG = "Download OTA";

extern const uint8_t rootca_crt_bundle_start[] asm("_binary_src_certs_x509_crt_bundle_bin_start");

static NotificationEvents *_notificationEvents = nullptr;
static int previousProgress = 0;
StaticJsonDocument<128> doc;

void update_started()
{
    String output;
    doc["status"] = "preparing";
    serializeJson(doc, output);
    _notificationEvents->send(output, "download_ota", millis());
}

void update_progress(int currentBytes, int totalBytes)
{
    String output;
    doc["status"] = "progress";
    int progress = ((currentBytes * 100) / totalBytes);
    if (progress > previousProgress)
    {
        doc["progress"] = progress;
        serializeJson(doc, output);
        _notificationEvents->send(output, "download_ota", millis());
        ESP_LOGV(TAG, "HTTP update process at %d of %d bytes... (%d %%)", currentBytes, totalBytes, progress);
    }
    previousProgress = progress;
}

void update_finished()
{
    String output;
    doc["status"] = "finished";
    serializeJson(doc, output);
    _notificationEvents->send(output, "download_ota", millis());

    // delay to allow the event to be sent out
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void updateTask(void *param)
{
    WiFiClientSecure client;
    client.setCACertBundle(rootca_crt_bundle_start);
    client.setTimeout(10);

    httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    httpUpdate.rebootOnUpdate(true);

    String url = *((String *)param);
    String output;
    httpUpdate.onStart(update_started);
    httpUpdate.onProgress(update_progress);
    httpUpdate.onEnd(update_finished);

    t_httpUpdate_return ret = httpUpdate.update(client, url.c_str());

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:

        doc["status"] = "error";
        doc["error"] = httpUpdate.getLastErrorString().c_str();
        serializeJson(doc, output);
        _notificationEvents->send(output, "download_ota", millis());

        Serial.printf("HTTP Update failed with error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;
    case HTTP_UPDATE_NO_UPDATES:

        doc["status"] = "error";
        doc["error"] = "Update failed, has same firmware version";
        serializeJson(doc, output);
        _notificationEvents->send(output, "download_ota", millis());

        Serial.println("HTTP Update failed, has same firmware version");
        break;
    case HTTP_UPDATE_OK:
        Serial.println("HTTP Update successful - Restarting");
        break;
    }
    vTaskDelete(NULL);
}

DownloadFirmwareService::DownloadFirmwareService(AsyncWebServer *server, SecurityManager *securityManager, NotificationEvents *notificationEvents) : _securityManager(securityManager),
                                                                                                                                                     _downloadHandler(GITHUB_FIRMWARE_PATH,
                                                                                                                                                                      _securityManager->wrapCallback(
                                                                                                                                                                          std::bind(&DownloadFirmwareService::downloadUpdate, this, std::placeholders::_1, std::placeholders::_2),
                                                                                                                                                                          AuthenticationPredicates::IS_ADMIN))

{
    _notificationEvents = notificationEvents;
    _downloadHandler.setMethod(HTTP_POST);
    _downloadHandler.setMaxContentLength(MAX_GITHUB_SIZE);
    server->addHandler(&_downloadHandler);
}

void DownloadFirmwareService::downloadUpdate(AsyncWebServerRequest *request, JsonVariant &json)
{
    String downloadURL = json["download_url"];
    Serial.println("Starting OTA from: " + downloadURL);

    doc["status"] = "preparing";
    doc["progress"] = 0;
    doc["error"] = "";

    String output;
    serializeJson(doc, output);
    _notificationEvents->send(output, "download_ota", millis());

    if (xTaskCreatePinnedToCore(
            &updateTask,                // Function that should be called
            "Update",                   // Name of the task (for debugging)
            OTA_TASK_STACK_SIZE,        // Stack size (bytes)
            &downloadURL,               // Pass reference to this class instance
            (configMAX_PRIORITIES - 1), // Pretty high task priority
            NULL,                       // Task handle
            1                           // Have it on application core
            ) != pdPASS)
    {
        ESP_LOGE(TAG, "Couldn't create download OTA task");
        request->send(500);
        return;
    }
    request->send(200);
}
