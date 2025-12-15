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

/**
 * This is github-io.pem
 */
const char *githubCACertificate = "-----BEGIN CERTIFICATE-----\n"
                                  "MIIGEzCCA/ugAwIBAgIQfVtRJrR2uhHbdBYLvFMNpzANBgkqhkiG9w0BAQwFADCB\n"
                                  "iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n"
                                  "cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n"
                                  "BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTgx\n"
                                  "MTAyMDAwMDAwWhcNMzAxMjMxMjM1OTU5WjCBjzELMAkGA1UEBhMCR0IxGzAZBgNV\n"
                                  "BAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEYMBYGA1UE\n"
                                  "ChMPU2VjdGlnbyBMaW1pdGVkMTcwNQYDVQQDEy5TZWN0aWdvIFJTQSBEb21haW4g\n"
                                  "VmFsaWRhdGlvbiBTZWN1cmUgU2VydmVyIENBMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
                                  "AQ8AMIIBCgKCAQEA1nMz1tc8INAA0hdFuNY+B6I/x0HuMjDJsGz99J/LEpgPLT+N\n"
                                  "TQEMgg8Xf2Iu6bhIefsWg06t1zIlk7cHv7lQP6lMw0Aq6Tn/2YHKHxYyQdqAJrkj\n"
                                  "eocgHuP/IJo8lURvh3UGkEC0MpMWCRAIIz7S3YcPb11RFGoKacVPAXJpz9OTTG0E\n"
                                  "oKMbgn6xmrntxZ7FN3ifmgg0+1YuWMQJDgZkW7w33PGfKGioVrCSo1yfu4iYCBsk\n"
                                  "Haswha6vsC6eep3BwEIc4gLw6uBK0u+QDrTBQBbwb4VCSmT3pDCg/r8uoydajotY\n"
                                  "uK3DGReEY+1vVv2Dy2A0xHS+5p3b4eTlygxfFQIDAQABo4IBbjCCAWowHwYDVR0j\n"
                                  "BBgwFoAUU3m/WqorSs9UgOHYm8Cd8rIDZsswHQYDVR0OBBYEFI2MXsRUrYrhd+mb\n"
                                  "+ZsF4bgBjWHhMA4GA1UdDwEB/wQEAwIBhjASBgNVHRMBAf8ECDAGAQH/AgEAMB0G\n"
                                  "A1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAbBgNVHSAEFDASMAYGBFUdIAAw\n"
                                  "CAYGZ4EMAQIBMFAGA1UdHwRJMEcwRaBDoEGGP2h0dHA6Ly9jcmwudXNlcnRydXN0\n"
                                  "LmNvbS9VU0VSVHJ1c3RSU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNybDB2Bggr\n"
                                  "BgEFBQcBAQRqMGgwPwYIKwYBBQUHMAKGM2h0dHA6Ly9jcnQudXNlcnRydXN0LmNv\n"
                                  "bS9VU0VSVHJ1c3RSU0FBZGRUcnVzdENBLmNydDAlBggrBgEFBQcwAYYZaHR0cDov\n"
                                  "L29jc3AudXNlcnRydXN0LmNvbTANBgkqhkiG9w0BAQwFAAOCAgEAMr9hvQ5Iw0/H\n"
                                  "ukdN+Jx4GQHcEx2Ab/zDcLRSmjEzmldS+zGea6TvVKqJjUAXaPgREHzSyrHxVYbH\n"
                                  "7rM2kYb2OVG/Rr8PoLq0935JxCo2F57kaDl6r5ROVm+yezu/Coa9zcV3HAO4OLGi\n"
                                  "H19+24rcRki2aArPsrW04jTkZ6k4Zgle0rj8nSg6F0AnwnJOKf0hPHzPE/uWLMUx\n"
                                  "RP0T7dWbqWlod3zu4f+k+TY4CFM5ooQ0nBnzvg6s1SQ36yOoeNDT5++SR2RiOSLv\n"
                                  "xvcRviKFxmZEJCaOEDKNyJOuB56DPi/Z+fVGjmO+wea03KbNIaiGCpXZLoUmGv38\n"
                                  "sbZXQm2V0TP2ORQGgkE49Y9Y3IBbpNV9lXj9p5v//cWoaasm56ekBYdbqbe4oyAL\n"
                                  "l6lFhd2zi+WJN44pDfwGF/Y4QA5C5BIG+3vzxhFoYt/jmPQT2BVPi7Fp2RBgvGQq\n"
                                  "6jG35LWjOhSbJuMLe/0CjraZwTiXWTb2qHSihrZe68Zk6s+go/lunrotEbaGmAhY\n"
                                  "LcmsJWTyXnW0OMGuf1pGg+pRyrbxmRE1a6Vqe8YAsOf4vmSyrcjC8azjUeqkk+B5\n"
                                  "yOGBQMkKW+ESPMFgKuOXwIlCypTPRpgSabuY0MLTDXJLR27lk8QyKGOHQ+SwMj4K\n"
                                  "00u/I5sUKUErmgQfky3xxzlIPK1aEn8=\n"
                                  "-----END CERTIFICATE-----\n";

static EventSocket *_socket = nullptr;
static int previousProgress = 0;
static String *otaURL = nullptr;
JsonDocument doc;

void update_started()
{
    String output;
    doc["status"] = "preparing";
    doc["progress"] = 0;
    doc["bytes_written"] = 0;
    doc["total_bytes"] = 0;
    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
    ESP_LOGI(SVK_TAG, "HTTP Update started");
#ifdef SERIAL_INFO
    Serial.println("HTTP Update started");
#endif
}

void update_progress(int currentBytes, int totalBytes)
{
    doc["status"] = "progress";
    int progress = ((currentBytes * 100) / totalBytes);
    if (progress > previousProgress)
    {
        doc["progress"] = progress;
        doc["bytes_written"] = currentBytes;
        doc["total_bytes"] = totalBytes;
        JsonObject jsonObject = doc.as<JsonObject>();
        _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
        ESP_LOGV(SVK_TAG, "HTTP update process at %d of %d bytes... (%d %%)", currentBytes, totalBytes, progress);
    }
    previousProgress = progress;
}

void update_finished()
{
    String output;
    doc["status"] = "finished";
    doc["progress"] = 100;
    // Keep the last known bytes_written and total_bytes from progress
    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
    ESP_LOGI(SVK_TAG, "HTTP Update successful - Restarting");
#ifdef SERIAL_INFO
    Serial.println("HTTP Update successful - Restarting");
#endif

    vTaskDelay(250 / portTICK_PERIOD_MS);
}

void updateTask(void *param)
{
    String url = *((String *)param);
    delete (String *)param; // Clean up the allocated memory

    WiFiClientSecure client;

#ifndef DOWNLOAD_OTA_SKIP_CERT_VERIFY

#if ESP_ARDUINO_VERSION_MAJOR == 3
    client.setCACertBundle(rootca_crt_bundle_start, rootca_crt_bundle_end - rootca_crt_bundle_start);
#else
    client.setCACertBundle(rootca_crt_bundle_start);
#endif

#else
    ESP_LOGW(SVK_TAG, "Skipping SSL certificate verification for OTA update!");
    client.setInsecure();
#endif

    client.setTimeout(12000);

    httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    httpUpdate.rebootOnUpdate(true);

    String output;
    httpUpdate.onStart(update_started);
    httpUpdate.onProgress(update_progress);
    httpUpdate.onEnd(update_finished);

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
    }

    if (_emitEvent)
    {
        jsonObject = doc.as<JsonObject>();
        _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
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

    if (!_socket->isEventValid(EVENT_OTA_UPDATE))
    {
        _socket->registerEvent(EVENT_OTA_UPDATE);
    }

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
    doc["bytes_written"] = 0;
    doc["total_bytes"] = 0;
    doc["error"] = "";

    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_OTA_UPDATE, jsonObject);

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
#ifdef SERIAL_INFO
        Serial.println("Couldn't create download OTA task");
#endif
        return request->reply(500);
    }
    return request->reply(200);
}
