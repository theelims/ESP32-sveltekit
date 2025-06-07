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

#include <CoreDump.h>
#include <esp32-hal.h>

#include "esp_core_dump.h"
#include "esp_partition.h"
#include "esp_flash.h"

#define MIN(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

CoreDump::CoreDump(PsychicHttpServer *server,
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

esp_err_t CoreDump::coreDump(PsychicRequest *request)
{
    size_t coredump_addr;
    size_t coredump_size;
    esp_err_t err = esp_core_dump_image_get(&coredump_addr, &coredump_size);
    if (err != ESP_OK)
    {
        request->reply(500, "application/json", "{\"status\":\"error\",\"message\":\"core dump not available\"}");
        return err;
    }
    size_t const chunk_len = 3 * 16; // must be multiple of 3
    size_t const b64_len = chunk_len / 3 * 4 + 4;
    uint8_t *const chunk = (uint8_t *)malloc(chunk_len);
    char *const b64 = (char *)malloc(b64_len);
    assert(chunk && b64);

    /*if (write_cfg->start) {
        if ((err = write_cfg->start(write_cfg->priv)) != ESP_OK) {
            return err;
        }
    }*/

    ESP_LOGI(SVK_TAG, "Coredump is %u bytes", coredump_size);
    httpd_resp_set_status(request->request(), "200 OK");
    PsychicResponse response(request);
    response.setCode(200);
    response.setContentType("text/plain");
    response.sendHeaders();
    for (size_t offset = 0; offset < coredump_size; offset += chunk_len)
    {
        uint const read_len = MIN(chunk_len, coredump_size - offset);
        if (esp_flash_read(esp_flash_default_chip, chunk, coredump_addr + offset, read_len))
        {
            ESP_LOGE(SVK_TAG, "Coredump read failed");
            break;
        }
        err = response.sendChunk(chunk, read_len);
        if (err != ESP_OK)
        {
            break;
        }
    }
    free(chunk);
    free(b64);

    err = response.finishChunking();

    /*uint32_t sec_num = coredump_size / SPI_FLASH_SEC_SIZE;
    if (coredump_size % SPI_FLASH_SEC_SIZE) {
        sec_num++;
    }
    err = esp_flash_erase_region(esp_flash_default_chip, coredump_addr, sec_num * SPI_FLASH_SEC_SIZE);
    if (err != ESP_OK) {
        ESP_LOGE(SVK_TAG, "Failed to erase coredump (%d)!", err);
    }*/
    return err;
}