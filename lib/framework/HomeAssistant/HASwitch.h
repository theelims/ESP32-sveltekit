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

#include <functional>
#include <StatefulService.h>
#include <HomeAssistant/HAActuatorEntity.h>

/**
 * @brief Home Assistant `switch` entity bound to one boolean field of a
 *        StatefulService<T> (thin wrapper around HAActuatorEntity<T>).
 *
 * Publishes "ON" / "OFF" state and accepts the same payloads as commands.
 * Multiple HASwitch<T> instances can share one StatefulService<T>, each
 * binding to a different bool field.
 *
 * Switch-specific discovery fields (optimistic, qos, retain, ...) can be
 * added via the optional ConfigBuilder lambda. Note: payload_on / payload_off
 * in the ConfigBuilder must match the "ON" / "OFF" strings that this wrapper
 * publishes and parses; to use different payloads, use HAActuatorEntity<T>
 * directly with custom serializer and applier lambdas.
 *
 * @code
 * auto sw = new HASwitch<MyState>(
 *     [](const MyState &s) { return s.enabled; },
 *     [](MyState &s, bool v) { s.enabled = v; },
 *     myService, haService, "enabled");
 * sw->setName("Enabled").setIcon("mdi:power");
 * haService->mainDevice().registerControl(std::unique_ptr<HASwitch<MyState>>(sw));
 * @endcode
 */
template <class T>
class HASwitch : public HAActuatorEntity<T>
{
public:
    using Getter = std::function<bool(const T &)>;
    using Setter = std::function<void(T &, bool)>;

    HASwitch(Getter getter,
             Setter setter,
             StatefulService<T> *statefulService,
             HAService *haService,
             const String &objectId,
             HAEntityBase::ConfigBuilder configBuilder = {})
        : HAActuatorEntity<T>(
              [getter](const T &s) -> String
              { return getter(s) ? "ON" : "OFF"; },
              [getter, setter](const String &cmd, T &s) -> StateUpdateResult
              {
                  bool newVal = (cmd == "ON");
                  if (getter(s) == newVal)
                      return StateUpdateResult::UNCHANGED;
                  setter(s, newVal);
                  return StateUpdateResult::CHANGED;
              },
              statefulService, haService, "switch", objectId, configBuilder)
    {
    }
};
