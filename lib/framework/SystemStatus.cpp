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

#include <SystemStatus.h>
#include <esp32-hal.h>

#if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
#include "esp32/rom/rtc.h"
#define ESP_PLATFORM "ESP32";
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32/rom/rtc.h"
#define ESP_PLATFORM "ESP32-S2";
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/rtc.h"
#define ESP_PLATFORM "ESP32-C3";
#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/rom/rtc.h"
#define ESP_PLATFORM "ESP32-S3";
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif

#ifndef ARDUINO_VERSION
#ifndef STRINGIZE
#define STRINGIZE(s) #s
#endif
#define ARDUINO_VERSION_STR(major, minor, patch) "v" STRINGIZE(major) "." STRINGIZE(minor) "." STRINGIZE(patch)
#define ARDUINO_VERSION ARDUINO_VERSION_STR(ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH)
#endif

String verbosePrintResetReason(int reason)
{
    switch (reason)
    {
    case 1:
        return ("Vbat power on reset");
        break;
    case 3:
        return ("Software reset digital core");
        break;
    case 4:
        return ("Legacy watch dog reset digital core");
        break;
    case 5:
        return ("Deep Sleep reset digital core");
        break;
    case 6:
        return ("Reset by SLC module, reset digital core");
        break;
    case 7:
        return ("Timer Group0 Watch dog reset digital core");
        break;
    case 8:
        return ("Timer Group1 Watch dog reset digital core");
        break;
    case 9:
        return ("RTC Watch dog Reset digital core");
        break;
    case 10:
        return ("Intrusion tested to reset CPU");
        break;
    case 11:
        return ("Time Group reset CPU");
        break;
    case 12:
        return ("Software reset CPU");
        break;
    case 13:
        return ("RTC Watch dog Reset CPU");
        break;
    case 14:
        return ("for APP CPU, reseted by PRO CPU");
        break;
    case 15:
        return ("Reset when the vdd voltage is not stable");
        break;
    case 16:
        return ("RTC Watch dog reset digital core and rtc module");
        break;
    default:
        return ("NO_MEAN");
    }
}

SystemStatus::SystemStatus(PsychicHttpServer *server,
                           SecurityManager *securityManager) : _server(server),
                                                               _securityManager(securityManager)
{
}

void SystemStatus::begin()
{
    _server->on(SYSTEM_STATUS_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&SystemStatus::systemStatus, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));

    ESP_LOGV("SystemStatus", "Registered GET endpoint: %s", SYSTEM_STATUS_SERVICE_PATH);
}

esp_err_t SystemStatus::systemStatus(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject root = response.getRoot();

    root["esp_platform"] = ESP_PLATFORM;
    root["firmware_version"] = APP_VERSION;
    root["max_alloc_heap"] = ESP.getMaxAllocHeap();
    root["psram_size"] = ESP.getPsramSize();
    root["free_psram"] = ESP.getFreePsram();
    root["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
    root["cpu_type"] = ESP.getChipModel();
    root["cpu_rev"] = ESP.getChipRevision();
    root["cpu_cores"] = ESP.getChipCores();
    root["free_heap"] = ESP.getFreeHeap();
    root["min_free_heap"] = ESP.getMinFreeHeap();
    root["sketch_size"] = ESP.getSketchSize();
    root["free_sketch_space"] = ESP.getFreeSketchSpace();
    root["sdk_version"] = ESP.getSdkVersion();
    root["arduino_version"] = ARDUINO_VERSION;
    root["flash_chip_size"] = ESP.getFlashChipSize();
    root["flash_chip_speed"] = ESP.getFlashChipSpeed();
    root["fs_total"] = ESPFS.totalBytes();
    root["fs_used"] = ESPFS.usedBytes();
    root["core_temp"] = temperatureRead();
    root["cpu_reset_reason"] = verbosePrintResetReason(rtc_get_reset_reason(0));
    root["uptime"] = millis() / 1000;

    return response.send();
}
