#pragma once

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

#include <JsonUtils.h>
#include <NotificationEvents.h>

class BatteryService
{
public:
    BatteryService(NotificationEvents *notificationEvents);

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
    NotificationEvents *_notificationEvents;
    int _lastSOC = 100;
    boolean _isCharging = false;
};
