#pragma once

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

#include <EventSocket.h>
#include <JsonUtils.h>

#define EVENT_BATTERY "battery"

class BatteryService
{
public:
    BatteryService(EventSocket *socket);

    void begin();

    void updateSOC(float stateOfCharge)
    {
        _lastSOC = (int)round(stateOfCharge);
        batteryEvent();
    }

    void setCharging(boolean isCharging)
    {
        _isCharging = isCharging;
        batteryEvent();
    }

private:
    void batteryEvent();
    EventSocket *_socket;
    int _lastSOC = 100;
    boolean _isCharging = false;
};
