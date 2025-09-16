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

#include <EnergyService.h>

EnergyService::EnergyService(EventSocket *socket) : _socket(socket)
{
}

void EnergyService::updateSOC(float stateOfCharge)
{
    _lastSOC = (int)round(stateOfCharge);
    energyEvent();
}

void EnergyService::setCharging(boolean isCharging)
{
    _isCharging = isCharging;
    energyEvent();
}

boolean EnergyService::isCharging()
{
    return _isCharging;
}

int EnergyService::getSOC()
{
    return _lastSOC;
}

void EnergyService::begin()
{
    _socket->registerEvent(EVENT_ENERGY);
}

void EnergyService::energyEvent()
{
    JsonDocument doc;
    doc["soc"] = _lastSOC;
    doc["charging"] = _isCharging;
    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_ENERGY, jsonObject);
}
