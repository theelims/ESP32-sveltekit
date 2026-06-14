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
#include <HomeAssistant/HAService.h>

HASensorEntity::HASensorEntity(HAService *haService,
                               const String &component,
                               const String &objectId,
                               ValueReader reader,
                               ConfigBuilder configBuilder)
    : HAEntityBase(haService, component, objectId),
      _reader(reader),
      _configBuilder(configBuilder)
{
}

void HASensorEntity::publishAll()
{
    if (_haService == nullptr || !_haService->isReady())
        return;

    JsonDocument config;
    config["state_topic"] = _stateTopicRel();
    config["unique_id"] = _getDeviceId() + "_" + _objectId;

    if (_configBuilder)
    {
        JsonObject obj = config.as<JsonObject>();
        _configBuilder(obj);
    }

    if (!_publishEntityConfig(config))
        return;

    publishState();
}

void HASensorEntity::publishState()
{
    if (_haService == nullptr || !_haService->isReady() || !_reader)
        return;

    _haService->publish(_stateTopicAbs(), _reader());
}
