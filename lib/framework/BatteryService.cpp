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

#include <BatteryService.h>

BatteryService::BatteryService(NotificationEvents *notificationEvents) : _notificationEvents(notificationEvents)
{
}

void BatteryService::batteryEvent()
{
    StaticJsonDocument<32> doc;
    String message;
    doc["soc"] = _lastSOC;
    doc["charging"] = _isCharging;
    serializeJson(doc, message);
    _notificationEvents->send(message, "battery", millis());
}
