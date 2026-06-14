/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2025 theelims
 *   Copyright (C) 2026 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <HomeAssistant/HADeviceIdentity.h>

void HADeviceIdentity::writeDeviceBlock(JsonDocument &doc) const
{
    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = id;
    device["name"] = name;

    if (!manufacturer.isEmpty())
        device["manufacturer"] = manufacturer;

    if (!model.isEmpty())
        device["model"] = model;

    if (!swVersion.isEmpty())
        device["sw_version"] = swVersion;

    if (!hwVersion.isEmpty())
        device["hw_version"] = hwVersion;

    if (!serialNumber.isEmpty())
        device["serial_number"] = serialNumber;

    if (!configurationUrl.isEmpty())
        device["configuration_url"] = configurationUrl;

    if (!viaDevice.isEmpty())
        device["via_device"] = viaDevice;

    if (!suggestedArea.isEmpty())
        device["suggested_area"] = suggestedArea;
}
