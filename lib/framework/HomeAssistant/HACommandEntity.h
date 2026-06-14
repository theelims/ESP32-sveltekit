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
 * @brief MQTT infrastructure base for command-only HA entities (button, trigger, scene).
 *
 * Publishes a discovery config with a command_topic and subscribes to it.
 * Invokes the provided Action on every incoming MQTT message.
 * Has no state — does not publish to a state_topic.
 *
 * Entity-specific HA discovery fields (e.g. payload_press for button) are
 * passed as a ConfigBuilder lambda to the constructor. The lambda runs after
 * the topology fields (command_topic, unique_id) have been set, so it can
 * add or override them.
 *
 * Common fields (name, icon, entity_category, ...) use the typed setters
 * inherited from HAEntityBase.
 */
class HACommandEntity : public HAEntityBase
{
public:
    static constexpr const char *TAG = "HACommandEntity";

    using Action = std::function<void()>;

    HACommandEntity(HAService *haService,
                    const String &component,
                    const String &objectId,
                    Action onPress,
                    ConfigBuilder configBuilder = {});

    void publishAll() override;

private:
    Action _onPress;
    ConfigBuilder _configBuilder;

    void _subscribe();
};
