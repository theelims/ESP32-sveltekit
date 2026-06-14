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

#include <HomeAssistant/HACommandEntity.h>
#include <HomeAssistant/HAService.h>

HACommandEntity::HACommandEntity(HAService *haService,
                                 const String &component,
                                 const String &objectId,
                                 Action onPress,
                                 ConfigBuilder configBuilder)
    : HAEntityBase(haService, component, objectId),
      _onPress(onPress),
      _configBuilder(configBuilder)
{
}

void HACommandEntity::publishAll()
{
    if (_haService == nullptr || !_haService->isReady())
        return;

    JsonDocument config;
    config["command_topic"] = _commandTopicRel();
    config["unique_id"] = _getDeviceId() + "_" + _objectId;

    if (_configBuilder)
    {
        JsonObject obj = config.as<JsonObject>();
        _configBuilder(obj);
    }

    if (!_publishEntityConfig(config))
        return;

    _subscribe();
}

void HACommandEntity::_subscribe()
{
    if (_haService == nullptr)
        return;

    // Capture _alive so the callback stays safe to invoke after this entity
    // is destroyed — HAService/PsychicMqttClient have no unsubscribe API, so
    // the lambda outlives the entity (e.g. when an alarm-line sub-device is
    // removed and re-added on import).
    auto alive = _alive;
    _haService->subscribe(_commandTopicAbs(),
                          [this, alive](char *, char *, int, int, bool)
                          {
                              if (*alive && _onPress)
                                  _onPress();
                          });
}
