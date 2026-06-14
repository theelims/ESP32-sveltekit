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

#include <HomeAssistant/HAEntityBase.h>
#include <HomeAssistant/HADevice.h>
#include <HomeAssistant/HAService.h>

HAEntityBase::HAEntityBase(HAService *haService,
                           const String &component,
                           const String &objectId)
    : _haService(haService),
      _ownerDevice(nullptr),
      _component(component),
      _objectId(objectId),
      _category(HACategory::Control),
      _enabledByDefault(false),
      _hasEnabledByDefault(false),
      _alive(std::make_shared<bool>(true))
{
}

HAEntityBase::~HAEntityBase()
{
    *_alive = false;
}

void HAEntityBase::begin()
{
    if (_haService == nullptr)
    {
        ESP_LOGW("HAEntityBase", "begin() called with null haService - %s will not publish", _objectId.c_str());
        return;
    }

    ESP_LOGI("HAEntityBase", "Registering %s/%s with onPublishAll", _component.c_str(), _objectId.c_str());
    // Capture _alive by value so the lambda stays safe to call after this
    // entity is destroyed — HAService has no deregistration API.
    auto alive = _alive;
    _haService->onPublishAll([this, alive]()
                             { if (*alive) this->publishAll(); });
}

// ============================================================================
// Typed common-field setters
// ============================================================================

HAEntityBase &HAEntityBase::setName(const String &name)
{
    _name = name;
    return *this;
}

HAEntityBase &HAEntityBase::setIcon(const String &icon)
{
    _icon = icon;
    return *this;
}

HAEntityBase &HAEntityBase::setEntityCategory(HACategory category)
{
    _category = category;
    return *this;
}

HAEntityBase &HAEntityBase::setDeviceClass(const String &deviceClass)
{
    _deviceClass = deviceClass;
    return *this;
}

HAEntityBase &HAEntityBase::setEntityPicture(const String &url)
{
    _entityPicture = url;
    return *this;
}

HAEntityBase &HAEntityBase::setObjectIdHint(const String &objectIdHint)
{
    _objectIdHint = objectIdHint;
    return *this;
}

HAEntityBase &HAEntityBase::setEnabledByDefault(bool enabled)
{
    _enabledByDefault = enabled;
    _hasEnabledByDefault = true;
    return *this;
}

HAEntityBase &HAEntityBase::setAvailabilityTopic(const String &topic)
{
    _availabilityTopic = topic;
    return *this;
}

HAEntityBase &HAEntityBase::setPayloadAvailable(const String &payload)
{
    _payloadAvailable = payload;
    return *this;
}

HAEntityBase &HAEntityBase::setPayloadNotAvailable(const String &payload)
{
    _payloadNotAvailable = payload;
    return *this;
}

HAEntityBase &HAEntityBase::setExtraConfig(ConfigBuilder builder)
{
    _extraConfig = builder;
    return *this;
}

// ============================================================================
// Topic helpers
// ============================================================================

String HAEntityBase::_getDeviceId() const
{
    if (_ownerDevice != nullptr)
        return _ownerDevice->getDeviceId();
    return (_haService != nullptr) ? _haService->getDeviceId() : String();
}

String HAEntityBase::_stateTopicRel() const
{
    return "~/" + _objectId + "/state";
}

String HAEntityBase::_commandTopicRel() const
{
    return "~/" + _objectId + "/set";
}

String HAEntityBase::_stateTopicAbs() const
{
    if (_ownerDevice != nullptr)
        return _ownerDevice->getBaseTopic() + "/" + _objectId + "/state";
    return _haService->getBaseTopic() + "/" + _objectId + "/state";
}

String HAEntityBase::_commandTopicAbs() const
{
    if (_ownerDevice != nullptr)
        return _ownerDevice->getBaseTopic() + "/" + _objectId + "/set";
    return _haService->getBaseTopic() + "/" + _objectId + "/set";
}

// ============================================================================
// Config publish
// ============================================================================

bool HAEntityBase::_publishEntityConfig(JsonDocument &config)
{
    // Apply typed common fields. Only write keys that are explicitly set —
    // an empty String / default enum means "not set" and the key is omitted.

    if (!_name.isEmpty())
        config["name"] = _name;

    if (!_icon.isEmpty())
        config["icon"] = _icon;

    switch (_category)
    {
    case HACategory::Config:
        config["entity_category"] = "config";
        break;
    case HACategory::Diagnostic:
        config["entity_category"] = "diagnostic";
        break;
    case HACategory::Control:
    default:
        // No entity_category key — HA defaults to "Controls".
        break;
    }

    if (!_deviceClass.isEmpty())
        config["device_class"] = _deviceClass;

    if (!_entityPicture.isEmpty())
        config["entity_picture"] = _entityPicture;

    if (!_objectIdHint.isEmpty())
        config["object_id"] = _objectIdHint;

    if (_hasEnabledByDefault)
        config["enabled_by_default"] = _enabledByDefault;

    if (!_availabilityTopic.isEmpty())
        config["availability_topic"] = _availabilityTopic;

    if (!_payloadAvailable.isEmpty())
        config["payload_available"] = _payloadAvailable;

    if (!_payloadNotAvailable.isEmpty())
        config["payload_not_available"] = _payloadNotAvailable;

    // Run the escape-hatch lambda last so it can override or extend.
    if (_extraConfig)
    {
        JsonObject obj = config.as<JsonObject>();
        _extraConfig(obj);
    }

    // Route through the owner device when registered via HADevice so that
    // sub-device entities use the correct base topic and identity block.
    if (_ownerDevice != nullptr)
        return _ownerDevice->publishConfig(_component, _objectId, config);

    return _haService->publishConfig(_component, _objectId, config);
}
