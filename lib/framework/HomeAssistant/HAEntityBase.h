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
#include <memory>

class HAService;
class HADevice;

/**
 * @brief HA "entity_category" classification.
 *
 * Maps to the optional MQTT-discovery `entity_category` field. Decides where
 * the entity appears on the HA device page:
 *   Control    → "Controls"        (no entity_category in payload)
 *   Config     → "Configuration"   (entity_category = "config")
 *   Diagnostic → "Diagnostic"      (entity_category = "diagnostic")
 */
enum class HACategory
{
    Control,
    Config,
    Diagnostic
};

/**
 * @brief Abstract base for Home Assistant entity endpoints.
 *
 * Holds the shared identity of an entity (component, objectId, common HA
 * fields) and registers the entity with HAService for publishing on MQTT
 * connect.
 *
 * Subclasses implement publishAll() to emit the discovery config, initial
 * state, and any command-topic subscriptions.
 *
 * Topic conventions used by subclasses:
 *   state   topic: ~/<objectId>/state
 *   command topic: ~/<objectId>/set
 * where ~ resolves to HAService::getBaseTopic().
 *
 * Common HA discovery fields are exposed as typed, chainable setters
 * (setName, setIcon, setEntityCategory, ...). The optional `extraConfig`
 * lambda is an escape hatch for niche or experimental fields not covered by
 * a typed setter — it runs after the typed values have been written, so it
 * can override them.
 */
class HAEntityBase
{
public:
    using ConfigBuilder = std::function<void(JsonObject &config)>;

    HAEntityBase(HAService *haService,
                 const String &component,
                 const String &objectId);

    virtual ~HAEntityBase();

    /**
     * Register with HAService::onPublishAll so that publishAll() runs on
     * every (re)connect to MQTT. Called automatically when the entity is
     * registered with an HADevice; existing standalone usage may call this
     * directly for backward compatibility.
     */
    void begin();

    /**
     * Publish discovery config (+ initial state, subscribe etc. as needed).
     * Subclass-defined.
     */
    virtual void publishAll() = 0;

    // ========================================================================
    // Typed common-field setters (chainable)
    //
    // Each setter records a single HA-discovery field. Empty/default values
    // mean "not set" and produce no key in the published config.
    // ========================================================================

    HAEntityBase &setName(const String &name);
    HAEntityBase &setIcon(const String &icon);
    HAEntityBase &setEntityCategory(HACategory category);
    HAEntityBase &setDeviceClass(const String &deviceClass);
    HAEntityBase &setEntityPicture(const String &url);
    HAEntityBase &setObjectIdHint(const String &objectIdHint);
    HAEntityBase &setEnabledByDefault(bool enabled);
    HAEntityBase &setAvailabilityTopic(const String &topic);
    HAEntityBase &setPayloadAvailable(const String &payload);
    HAEntityBase &setPayloadNotAvailable(const String &payload);

    /**
     * Set an arbitrary-fields lambda. Runs after the typed setters have been
     * applied, so it can override or extend them. Use only for fields not
     * yet covered by a typed setter.
     */
    HAEntityBase &setExtraConfig(ConfigBuilder builder);

    // ========================================================================
    // Internal — set by HADevice when the entity is registered.
    // ========================================================================
    void _setOwnerDevice(HADevice *device) { _ownerDevice = device; }

    const String &component() const { return _component; }
    const String &objectId() const { return _objectId; }

protected:
    HAService *_haService;
    HADevice *_ownerDevice;
    String _component;
    String _objectId;

    // Typed common fields
    String _name;
    String _icon;
    HACategory _category;
    String _deviceClass;
    String _entityPicture;
    String _objectIdHint;
    String _availabilityTopic;
    String _payloadAvailable;
    String _payloadNotAvailable;
    bool _enabledByDefault;
    bool _hasEnabledByDefault;

    // Escape hatch for fields not covered by typed setters
    ConfigBuilder _extraConfig;

    // Liveness flag captured by the lambda registered with HAService in begin().
    // Set to false in the destructor so post-destruction invocations bail out
    // instead of dereferencing freed memory — HAService never deregisters its
    // _publishCallbacks, so the lambda can outlive us.
    std::shared_ptr<bool> _alive;

    /**
     * Returns the device ID that should appear in entity `unique_id` values.
     * Uses the owner device's ID when registered via HADevice, otherwise falls
     * back to the main device ID from HAService.
     */
    String _getDeviceId() const;

    /** "~/<objectId>/state" — relative form for use inside discovery configs. */
    String _stateTopicRel() const;

    /** "~/<objectId>/set" — relative form for use inside discovery configs. */
    String _commandTopicRel() const;

    /**
     * Absolute state topic. Resolves through the owner device's base topic
     * when registered via HADevice, otherwise through HAService.
     */
    String _stateTopicAbs() const;

    /**
     * Absolute command topic. Resolves through the owner device's base topic
     * when registered via HADevice, otherwise through HAService.
     */
    String _commandTopicAbs() const;

    /**
     * Apply typed common-field values to @p config, then run the extraConfig
     * lambda (if any), then publish via HAService. The caller is expected to
     * have pre-populated entity-specific topology fields (state_topic,
     * command_topic, unique_id, ...) so the typed setters and extraConfig
     * can override or extend them.
     */
    bool _publishEntityConfig(JsonDocument &config);
};
