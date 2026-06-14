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

#include <StatefulService.h>
#include <HomeAssistant/HAActuatorEntity.h>
#include <HomeAssistant/HAService.h>

/**
 * @brief Home Assistant `light` entity bound to a StatefulService<T>
 *        (thin wrapper around HAActuatorEntity<T>).
 *
 * Uses the same JsonStateReader<T> / JsonStateUpdater<T> pair as
 * HttpEndpoint<T> and MqttEndpoint<T> to serialise and deserialise state.
 * State is published as a JSON string; commands are received as JSON strings.
 *
 * Light-specific discovery fields (schema, brightness, supported_color_modes,
 * color_mode, ...) are passed via the optional ConfigBuilder lambda:
 *
 * @code
 * auto light = new HALight<LightState>(
 *     LightState::haRead, LightState::haUpdate, this, haService, "led",
 *     [](JsonObject &c) {
 *         c["schema"]     = "json";
 *         c["brightness"] = true;
 *     });
 * light->setName("LED").setIcon("mdi:led-on");
 * haService->mainDevice().registerControl(std::unique_ptr<HALight<LightState>>(light));
 * @endcode
 */
template <class T>
class HALight : public HAActuatorEntity<T>
{
public:
    HALight(JsonStateReader<T> stateReader,
            JsonStateUpdater<T> stateUpdater,
            StatefulService<T> *statefulService,
            HAService *haService,
            const String &objectId,
            HAEntityBase::ConfigBuilder configBuilder = {})
        : HAActuatorEntity<T>(
              [stateReader](const T &s) -> String
              {
                  JsonDocument doc;
                  JsonObject obj = doc.to<JsonObject>();
                  stateReader(const_cast<T &>(s), obj);
                  String payload;
                  serializeJson(doc, payload);
                  return payload;
              },
              [stateUpdater](const String &cmd, T &s) -> StateUpdateResult
              {
                  JsonDocument doc;
                  if (deserializeJson(doc, cmd) != DeserializationError::Ok)
                      return StateUpdateResult::UNCHANGED;
                  JsonObject obj = doc.as<JsonObject>();
                  return stateUpdater(obj, s, HA_ORIGIN_ID);
              },
              statefulService, haService, "light", objectId, configBuilder)
    {
    }
};
