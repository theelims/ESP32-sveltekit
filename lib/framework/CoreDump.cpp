/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <CoreDump.h>
#include <esp32-hal.h>

#include "esp_core_dump.h"
#include "esp_partition.h"
#include "esp_flash.h"

CoreDump::CoreDump(AsyncWebServer *server,
                   SecurityManager *securityManager) : _server(server),
                                                       _securityManager(securityManager)
{
}

void CoreDump::begin()
{
    _server->on(CORE_DUMP_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&CoreDump::coreDump, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));

    ESP_LOGV("CoreDump", "Registered GET endpoint: %s", CORE_DUMP_SERVICE_PATH);
}

void CoreDump::coreDump(AsyncWebServerRequest *request)
{
    size_t coredump_addr;
    size_t coredump_size;
    esp_err_t err = esp_core_dump_image_get(&coredump_addr, &coredump_size);
    if (err != ESP_OK)
    {
        request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"core dump not available\"}");
        return;
    }
    ESP_LOGI(SVK_TAG, "Coredump is %u bytes", coredump_size);

    uint8_t *buffer = (uint8_t *)malloc(coredump_size);
    if (!buffer)
    {
        request->send(500, "text/plain", "coredump alloc failed");
        return;
    }
    err = esp_flash_read(esp_flash_default_chip, buffer, coredump_addr, coredump_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(SVK_TAG, "Coredump read failed");
        free(buffer);
        request->send(500, "text/plain", "coredump read failed");
        return;
    }
    AsyncWebServerResponse *response = request->beginResponse(200, "application/octet-stream", buffer, coredump_size);
    request->send(response);
    free(buffer);
}