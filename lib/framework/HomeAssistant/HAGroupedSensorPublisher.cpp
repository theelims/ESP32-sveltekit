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

#include <HomeAssistant/HAGroupedSensorPublisher.h>
#include <HomeAssistant/HADevice.h>

HAGroupedSensorPublisher::HAGroupedSensorPublisher(HAService *haService,
                                                   const String &topicSuffix,
                                                   StateReader stateReader,
                                                   HADevice *ownerDevice)
    : _haService(haService),
      _ownerDevice(ownerDevice),
      _topicSuffix(topicSuffix),
      _stateReader(stateReader),
      _alive(std::make_shared<bool>(true))
{
}

HAGroupedSensorPublisher::~HAGroupedSensorPublisher()
{
    *_alive = false;
}

HAGroupedSensorPublisher &HAGroupedSensorPublisher::addSensor(const String &component,
                                                              const String &objectId,
                                                              const String &name,
                                                              const String &icon,
                                                              HACategory category,
                                                              ConfigBuilder extraConfig)
{
    _sensors.push_back({component, objectId, name, icon, category, extraConfig});
    return *this;
}

void HAGroupedSensorPublisher::begin()
{
    if (_haService == nullptr)
        return;

    // Capture _alive by value so the lambdas stay safe to call after this
    // publisher is destroyed — HAService has no deregistration API.
    auto alive = _alive;
    _haService->onPublishAll([this, alive]() {
        if (*alive) this->publishAll();
    });
    _haService->onUnpublishAll([this, alive]() {
        if (*alive) this->unpublishAll();
    });
}

void HAGroupedSensorPublisher::publishAll()
{
    if (_haService == nullptr || !_haService->isReady())
        return;

    for (const auto &sensor : _sensors)
    {
        _publishSensorConfig(sensor);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    publishState();
}

void HAGroupedSensorPublisher::publishState()
{
    if (_haService == nullptr || !_haService->isReady())
        return;

    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    if (_stateReader)
    {
        _stateReader(root);
    }

    String payload;
    serializeJson(doc, payload);
    _haService->publish(_sharedStateTopicAbs(), payload);
}

String HAGroupedSensorPublisher::_deviceId() const
{
    if (_ownerDevice != nullptr)
        return _ownerDevice->getDeviceId();
    return _haService->getDeviceId();
}

String HAGroupedSensorPublisher::_sharedStateTopicRel() const
{
    return "~/" + _topicSuffix + "/state";
}

String HAGroupedSensorPublisher::_sharedStateTopicAbs() const
{
    if (_ownerDevice != nullptr)
        return _ownerDevice->getBaseTopic() + "/" + _topicSuffix + "/state";
    return _haService->getBaseTopic() + "/" + _topicSuffix + "/state";
}

void HAGroupedSensorPublisher::unpublishAll()
{
    if (_haService == nullptr || !_haService->isReady())
        return;

    _haService->publish(_sharedStateTopicAbs(), String(), 0, true, false);

    String id = _deviceId();
    for (const auto &sensor : _sensors)
    {
        String configTopic = _haService->getDiscoveryPrefix() + sensor.component +
                             "/" + id + "/" + sensor.objectId + "/config";
        _haService->publish(configTopic, String(), 0, true, false);
    }
}

void HAGroupedSensorPublisher::_publishSensorConfig(const Sensor &sensor)
{
    JsonDocument config;
    config["state_topic"] = _sharedStateTopicRel();
    config["unique_id"]   = _deviceId() + "_" + sensor.objectId;

    if (!sensor.name.isEmpty())
        config["name"] = sensor.name;

    if (!sensor.icon.isEmpty())
        config["icon"] = sensor.icon;

    switch (sensor.category)
    {
    case HACategory::Config:
        config["entity_category"] = "config";
        break;
    case HACategory::Diagnostic:
        config["entity_category"] = "diagnostic";
        break;
    case HACategory::Control:
    default:
        break;
    }

    if (sensor.extraConfig)
    {
        JsonObject obj = config.as<JsonObject>();
        sensor.extraConfig(obj);
    }

    if (_ownerDevice != nullptr)
        _ownerDevice->publishConfig(sensor.component, sensor.objectId, config);
    else
        _haService->publishConfig(sensor.component, sensor.objectId, config);
}
