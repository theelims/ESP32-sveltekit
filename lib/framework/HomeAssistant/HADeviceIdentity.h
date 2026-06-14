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

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Identity block for one Home Assistant MQTT device.
 *
 * Holds the per-device fields that get serialized into the `device` block of
 * every entity discovery payload (identifiers, name, manufacturer, model,
 * sw_version, hw_version, configuration_url, via_device).
 *
 * Used by HADevice to author the device block, and by HAService::mainDevice
 * to expose the framework's primary device. Sub-devices (e.g., one per
 * remote sensor) populate their own HADeviceIdentity.
 *
 * `topicNamespace` is the slugified namespace used inside the MQTT base
 * topic — it does NOT appear in the discovery payload, only in topic paths.
 */
class HADeviceIdentity
{
public:
    String id;               ///< Unique device identifier (used in topic + identifiers)
    String name;             ///< Human-readable device name
    String manufacturer;     ///< Optional
    String model;            ///< Optional
    String swVersion;        ///< Optional
    String hwVersion;        ///< Optional
    String configurationUrl; ///< Optional — typically "http://<ip>/"
    String serialNumber;     ///< Optional — shown in HA device info panel
    String viaDevice;        ///< Optional — identifier of bridge/parent device
    String suggestedArea;    ///< Optional — HA room/area suggestion (e.g., "Living Room")
    String topicNamespace;   ///< Slugified, used in topic paths (not in payload)

    /**
     * @brief Write the `device` JSON block into a discovery config document.
     *
     * Adds `device.identifiers[0] = id`, `device.name = name`, and any of the
     * optional fields that are non-empty. Idempotent — safe to call on a
     * document that already has a `device` key (overwrites it).
     */
    void writeDeviceBlock(JsonDocument &doc) const;
};
