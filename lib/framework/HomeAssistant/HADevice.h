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
#include <type_traits>
#include <vector>

#include <HomeAssistant/HADeviceIdentity.h>
#include <HomeAssistant/HAEntityBase.h>

class HAService;

/**
 * @brief A Home Assistant MQTT device — owns identity + the entities under it.
 *
 * HA models everything as devices, each with a set of entities grouped on the
 * device's HA page into Controls / Configuration / Diagnostic sections. This
 * class mirrors that structure: each HADevice carries its own
 * HADeviceIdentity and owns a list of entities, registered via the three
 * register* methods that map to HA's `entity_category`:
 *
 *   registerControl(...)    → entity appears under Controls
 *   registerConfig(...)     → entity_category = "config"
 *   registerDiagnostic(...) → entity_category = "diagnostic"
 *
 * Entities are passed in as unique_ptr — HADevice owns them and unpublishes
 * them on destruction.
 *
 * The framework's primary device is reachable via HAService::mainDevice().
 * Sub-devices (e.g., one per discovered remote node in gateway projects)
 * are added via HAService::addSubDevice() and removed via
 * HAService::removeSubDevice() at runtime.
 *
 * Each sub-device publishes its own MQTT discovery block using its own
 * identity, with topics nested under the main device's base topic:
 *   {mainBaseTopic}/{subDeviceId}/{entityId}/state
 *
 * Usage (sub-device):
 * @code
 * HADeviceIdentity id;
 * id.id   = "gateway-aabbcc-node-01";
 * id.name = "Node 01";
 * id.suggestedArea = "Living Room";
 *
 * auto dev = std::make_unique<HADevice>(haService, std::move(id));
 * auto btn = std::make_unique<HAButton>(haService, "reset", []() { ... });
 * btn->setName("Reset Node");
 * dev->registerControl(std::move(btn));
 *
 * HADevice *raw = haService->addSubDevice(std::move(dev));
 * // raw is valid until haService->removeSubDevice(id.id) is called.
 * @endcode
 */
class HADevice
{
public:
    HADevice(HAService *haService, HADeviceIdentity identity);

    HADevice(const HADevice &) = delete;
    HADevice &operator=(const HADevice &) = delete;

    // ========================================================================
    // Identity
    // ========================================================================

    HADeviceIdentity &identity() { return _identity; }
    const HADeviceIdentity &identity() const { return _identity; }

    /** @brief Unique device identifier (HADeviceIdentity::id). */
    const String &getDeviceId() const { return _identity.id; }

    /**
     * @brief MQTT base topic for this device's entities.
     *
     * For the main device:  {discoveryPrefix}{namespace}/{deviceId}
     * For a sub-device:     {mainDevice.baseTopic}/{subDeviceId}
     *
     * Entities store state at {baseTopic}/{objectId}/state and listen on
     * {baseTopic}/{objectId}/set.
     */
    String getBaseTopic() const;

    // ========================================================================
    // Entity registration
    // ========================================================================

    /**
     * @brief Register an entity under this device's Controls section.
     *
     * Sets entity_category = Control (no entity_category key in payload — HA
     * default), takes ownership of the entity.
     *
     * @return raw pointer valid while the HADevice owns it. Useful for
     *         chaining setName/setIcon after move.
     */
    template <class E>
    E *registerControl(std::unique_ptr<E> entity)
    {
        return _registerAs(std::move(entity), HACategory::Control);
    }

    /** @brief Register under Configuration (entity_category = "config"). */
    template <class E>
    E *registerConfig(std::unique_ptr<E> entity)
    {
        return _registerAs(std::move(entity), HACategory::Config);
    }

    /** @brief Register under Diagnostic (entity_category = "diagnostic"). */
    template <class E>
    E *registerDiagnostic(std::unique_ptr<E> entity)
    {
        return _registerAs(std::move(entity), HACategory::Diagnostic);
    }

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /**
     * @brief Publish discovery configs + initial state for all entities.
     *
     * Called by HAService on every MQTT (re)connect. Also called immediately
     * by addSubDevice() when MQTT is already connected.
     */
    void publishAll();

    /**
     * @brief Remove this device from HA by sending empty retained payloads.
     *
     * The destructor does NOT call this automatically — on firmware restart
     * we want HA to keep the retained device config. Call explicitly before
     * destroying a sub-device that should disappear from HA.
     */
    void unpublishAll();

    /** @brief Number of entities registered with this device. */
    size_t entityCount() const { return _entities.size(); }

    // ========================================================================
    // MQTT helpers — used by entities owned by this device
    // ========================================================================

    /**
     * @brief Publish a discovery config for an entity on this device.
     *
     * Sets "~" to this device's base topic, attaches this device's identity
     * block, and publishes to:
     *   {discoveryPrefix}{component}/{deviceId}/{objectId}/config
     *
     * Used by entities to route through their owner device instead of the
     * main HAService device.
     */
    bool publishConfig(const String &component,
                       const String &objectId,
                       JsonDocument &config);

    /**
     * @brief Convenience wrapper — delegates to HAService::publish().
     */
    bool publish(const String &topic,
                 const String &payload,
                 int qos    = 0,
                 bool retain = true,
                 bool async  = true);

private:
    HAService *_haService;
    HADeviceIdentity _identity;
    std::vector<std::unique_ptr<HAEntityBase>> _entities;

    template <class E>
    E *_registerAs(std::unique_ptr<E> entity, HACategory category)
    {
        static_assert(std::is_base_of<HAEntityBase, E>::value,
                      "registered type must derive from HAEntityBase");

        if (!entity)
            return nullptr;

        E *raw = entity.get();
        raw->setEntityCategory(category);
        raw->_setOwnerDevice(this);
        _entities.push_back(std::move(entity));
        return raw;
    }
};
