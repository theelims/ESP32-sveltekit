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
#include <StatefulService.h>
#include <HomeAssistant/HAEntityBase.h>
#include <HomeAssistant/HAService.h>

/**
 * @brief MQTT infrastructure base for bidirectional HA entities (switch, light, cover, ...).
 *
 * Maintains a state_topic (published on connect and on every StatefulService
 * update) and a command_topic (subscribed — incoming payloads are applied back
 * to the service).
 *
 * State serialisation and command deserialisation are fully delegated to the
 * caller via two lambdas, keeping this class free of any HA payload format
 * knowledge:
 *
 *   StateSerializer  T → String       called whenever state needs publishing
 *   CommandApplier   String × T → StateUpdateResult
 *                                     called when an MQTT command arrives;
 *                                     must return CHANGED or UNCHANGED
 *
 * Entity-specific HA discovery fields (payload_on/off, schema, brightness, ...)
 * are passed as a ConfigBuilder lambda to the constructor. The lambda runs
 * after the topology fields (state_topic, command_topic, unique_id) have been
 * set, so it can add or override them.
 *
 * Common fields (name, icon, entity_category, ...) use the typed setters
 * inherited from HAEntityBase.
 */
template <class T>
class HAActuatorEntity : public HAEntityBase
{
public:
    static constexpr const char *TAG = "HAActuatorEntity";

    using StateSerializer = std::function<String(const T &)>;
    using CommandApplier  = std::function<StateUpdateResult(const String &, T &)>;

    HAActuatorEntity(StateSerializer serializer,
                     CommandApplier applier,
                     StatefulService<T> *statefulService,
                     HAService *haService,
                     const String &component,
                     const String &objectId,
                     ConfigBuilder configBuilder = {})
        : HAEntityBase(haService, component, objectId),
          _serializer(serializer),
          _applier(applier),
          _statefulService(statefulService),
          _configBuilder(configBuilder)
    {
        if (_statefulService != nullptr)
        {
            _statefulService->addUpdateHandler(
                [this](const String &) { _publishState(); },
                false);
        }
    }

    void publishAll() override
    {
        if (_haService == nullptr || !_haService->isReady())
            return;

        JsonDocument config;
        config["state_topic"]   = _stateTopicRel();
        config["command_topic"] = _commandTopicRel();
        config["unique_id"]     = _getDeviceId() + "_" + _objectId;

        if (_configBuilder)
        {
            JsonObject obj = config.as<JsonObject>();
            _configBuilder(obj);
        }

        if (!_publishEntityConfig(config))
            return;

        _publishState();
        _subscribe();
    }

private:
    StateSerializer    _serializer;
    CommandApplier     _applier;
    StatefulService<T> *_statefulService;
    ConfigBuilder      _configBuilder;

    void _publishState()
    {
        if (_haService == nullptr || !_haService->isReady()
            || _statefulService == nullptr || !_serializer)
            return;

        String state;
        _statefulService->read([&](const T &s) { state = _serializer(s); });
        _haService->publish(_stateTopicAbs(), state);
    }

    void _subscribe()
    {
        if (_haService == nullptr)
            return;

        // Capture _alive so the callback stays safe to invoke after this
        // entity is destroyed — HAService/PsychicMqttClient have no
        // unsubscribe API, so the lambda outlives the entity.
        auto alive = _alive;
        _haService->subscribe(_commandTopicAbs(),
                              [this, alive](char *, char *payload, int, int, bool)
                              {
                                  if (*alive && payload != nullptr)
                                      _onCommand(payload);
                              });
    }

    void _onCommand(const char *rawPayload)
    {
        if (_statefulService == nullptr || !_applier)
            return;

        String cmd(rawPayload);
        _statefulService->update(
            [this, cmd](T &s) -> StateUpdateResult
            { return _applier(cmd, s); },
            HA_ORIGIN_ID);
    }
};
