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

#include <SystemStatus.h>
#include <esp32-hal.h>

#if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
#include "esp32/rom/rtc.h"
#define ESP_TARGET "ESP32";
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32/rom/rtc.h"
#define ESP_TARGET "ESP32-S2";
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/rtc.h"
#define ESP_TARGET "ESP32-C3";
#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/rom/rtc.h"
#define ESP_TARGET "ESP32-S3";
#elif CONFIG_IDF_TARGET_ESP32C6
#include "esp32c6/rom/rtc.h"
#define ESP_TARGET "ESP32-C6";
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
    case ESP_RST_UNKNOWN:
        return ("Reset reason can not be determined");
        break;
    case ESP_RST_POWERON:
        return ("Reset due to power-on event");
        break;
    case ESP_RST_EXT:
        return ("Reset by external pin (not applicable for ESP32)");
        break;
    case ESP_RST_SW:
        return ("Software reset via esp_restart");
        break;
    case ESP_RST_PANIC:
        return ("Software reset due to exception/panic");
        break;
    case ESP_RST_INT_WDT:
        return ("Reset (software or hardware) due to interrupt watchdog");
        break;
    case ESP_RST_TASK_WDT:
        return ("Reset due to task watchdog");
        break;
    case ESP_RST_WDT:
        return ("Reset due to other watchdogs");
        break;
    case ESP_RST_DEEPSLEEP:
        return ("Reset after exiting deep sleep mode");
        break;
    case ESP_RST_BROWNOUT:
        return ("Brownout reset (software or hardware)");
        break;
    case ESP_RST_SDIO:
        return ("Reset over SDIO");
        break;
#ifdef ESP_RST_USB
    case ESP_RST_USB:
        return ("Reset by USB peripheral");
        break;
#endif
#ifdef ESP_RST_JSVK_TAG
    case ESP_RST_JSVK_TAG:
        return ("Reset by JSVK_TAG");
        break;
#endif
#ifdef ESP_RST_EFUSE
    case ESP_RST_EFUSE:
        return ("Reset due to efuse error");
        break;
#endif
#ifdef ESP_RST_PWR_GLITCH
    case ESP_RST_PWR_GLITCH:
        return ("Reset due to power glitch detected");
        break;
#endif
#ifdef ESP_RST_CPU_LOCKUP
    case ESP_RST_CPU_LOCKUP:
        return ("Reset due to CPU lock up (double exception)");
        break;
#endif
    default:
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "Unknown reset reason (%d)", reason);
        return String(buffer);
        break;
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

    ESP_LOGV(SVK_TAG, "Registered GET endpoint: %s", SYSTEM_STATUS_SERVICE_PATH);
}

esp_err_t SystemStatus::systemStatus(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject root = response.getRoot();

    root["esp_platform"] = ESP_TARGET;
    root["firmware_version"] = APP_VERSION;
    root["max_alloc_heap"] = ESP.getMaxAllocHeap();
    if (psramFound())
    {
        root["free_psram"] = ESP.getFreePsram();
        root["used_psram"] = ESP.getPsramSize() - ESP.getFreePsram();
        root["psram_size"] = ESP.getPsramSize();
    }
    root["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
    root["cpu_type"] = ESP.getChipModel();
    root["cpu_rev"] = ESP.getChipRevision();
    root["cpu_cores"] = ESP.getChipCores();
    root["free_heap"] = ESP.getFreeHeap();
    root["used_heap"] = ESP.getHeapSize() - ESP.getFreeHeap();
    root["total_heap"] = ESP.getHeapSize();
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
    root["cpu_reset_reason"] = verbosePrintResetReason(esp_reset_reason());
    root["uptime"] = millis() / 1000;

    return response.send();
}
