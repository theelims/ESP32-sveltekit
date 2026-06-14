#pragma once

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

#include <HomeAssistant/HASensorEntity.h>

/**
 * @brief Home Assistant `binary_sensor` entity (thin wrapper around HASensorEntity).
 *
 * Sets component = "binary_sensor". Accepts a bool-returning ValueReader and
 * converts it to "ON" / "OFF" strings for MQTT.
 *
 * Binary-sensor-specific discovery fields (device_class, payload_on/off,
 * value_template, ...) are passed via the optional ConfigBuilder lambda.
 * Note: if you supply custom payload_on / payload_off values in the
 * ConfigBuilder, you must also change the ValueReader to return the matching
 * strings by using HASensorEntity directly instead.
 *
 * @code
 * auto motion = new HABinarySensor(haService, "motion",
 *     []() { return motionDetected(); },
 *     [](JsonObject &c) { c["device_class"] = "motion"; });
 * motion->setName("Motion");
 * haService->mainDevice().registerControl(std::unique_ptr<HABinarySensor>(motion));
 * @endcode
 */
class HABinarySensor : public HASensorEntity
{
public:
    using ValueReader = std::function<bool()>;

    HABinarySensor(HAService *haService,
                   const String &objectId,
                   ValueReader reader,
                   ConfigBuilder configBuilder = {})
        : HASensorEntity(haService, "binary_sensor", objectId,
                         [reader]() -> String
                         { return reader() ? "ON" : "OFF"; },
                         configBuilder)
    {
    }
};
