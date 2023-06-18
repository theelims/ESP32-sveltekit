/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <SystemStatus.h>
#include <esp32-hal.h>

SystemStatus::SystemStatus(AsyncWebServer *server, SecurityManager *securityManager)
{
    server->on(SYSTEM_STATUS_SERVICE_PATH,
               HTTP_GET,
               securityManager->wrapRequest(std::bind(&SystemStatus::systemStatus, this, std::placeholders::_1),
                                            AuthenticationPredicates::IS_AUTHENTICATED));
}

void SystemStatus::systemStatus(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse(false, MAX_ESP_STATUS_SIZE);
    JsonObject root = response->getRoot();
    root["esp_platform"] = "esp32";
    root["max_alloc_heap"] = ESP.getMaxAllocHeap();
    root["psram_size"] = ESP.getPsramSize();
    root["free_psram"] = ESP.getFreePsram();
    root["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
    root["free_heap"] = ESP.getFreeHeap();
    root["sketch_size"] = ESP.getSketchSize();
    root["free_sketch_space"] = ESP.getFreeSketchSpace();
    root["sdk_version"] = ESP.getSdkVersion();
    root["flash_chip_size"] = ESP.getFlashChipSize();
    root["flash_chip_speed"] = ESP.getFlashChipSpeed();
    root["fs_total"] = ESPFS.totalBytes();
    root["fs_used"] = ESPFS.usedBytes();
    root["core_temp"] = temperatureRead();
    response->setLength();
    request->send(response);
}
