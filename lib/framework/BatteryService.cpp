/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <BatteryService.h>

BatteryService::BatteryService(EventSocket *socket) : _socket(socket)
{
}

void BatteryService::begin()
{
    _socket->registerEvent(EVENT_BATTERY);
}

void BatteryService::batteryEvent()
{
    JsonDocument doc;
    doc["soc"] = _lastSOC;
    doc["charging"] = _isCharging;
    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_BATTERY, jsonObject);
}
