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
#include <functional>
#include <HomeAssistant/HAEntityBase.h>

/**
 * @brief MQTT infrastructure base for state-only HA entities (sensor, binary_sensor, event).
 *
 * Publishes a discovery config with a state_topic. Has no command_topic —
 * HA cannot send commands to this entity.
 *
 * State is published on every MQTT (re)connect via publishAll(), and can be
 * refreshed at any time by calling publishState() — typically from a periodic
 * timer or when the underlying value changes.
 *
 * The ValueReader lambda returns the current value as a String and is called
 * every time the state needs publishing. Conversions (e.g. bool → "ON"/"OFF",
 * float → String) are the caller's responsibility.
 *
 * Entity-specific HA discovery fields (unit_of_measurement, device_class,
 * state_class, value_template, payload_on/off, ...) are passed as a
 * ConfigBuilder lambda to the constructor.
 *
 * Common fields (name, icon, entity_category, ...) use the typed setters
 * inherited from HAEntityBase.
 */
class HASensorEntity : public HAEntityBase
{
public:
    static constexpr const char *TAG = "HASensorEntity";

    using ValueReader = std::function<String()>;

    HASensorEntity(HAService *haService,
                   const String &component,
                   const String &objectId,
                   ValueReader reader,
                   ConfigBuilder configBuilder = {});

    void publishAll() override;

    /** Re-publish current state without re-sending the discovery config. */
    void publishState();

private:
    ValueReader _reader;
    ConfigBuilder _configBuilder;
};
