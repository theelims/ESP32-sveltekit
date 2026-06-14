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
#include <vector>
#include <HomeAssistant/HAService.h>
#include <HomeAssistant/HAEntityBase.h>

class HADevice;

/**
 * @brief Multi-sensor publisher sharing one MQTT state topic.
 *
 * A group of HA sensor entities (or binary_sensor) share a single state
 * topic — `~/<topicSuffix>/state` — that carries a JSON object with all
 * values. Each entity selects its value from the JSON via its own
 * `value_template`.
 *
 * Pull-based: the supplied StateReader lambda is invoked whenever the state
 * is published. Use this for live-measured values (heap, temperature, RSSI,
 * uptime, ...) that are read on demand. Trigger publishes via
 * publishState() — typically from a periodic timer.
 *
 * Note: this does not derive from HAEntityBase because HAEntityBase models a
 * single entity (1 component / 1 objectId). A grouped publisher has N entity
 * configs over one shared state topic, so the shapes do not align — the only
 * commonality is the HAService::onPublishAll registration, duplicated here
 * in a few lines.
 *
 * Usage:
 * @code
 * HAGroupedSensorPublisher _diag(haService, "diagnostics",
 *     [](JsonObject &state) {
 *         state["state"] = "online";
 *         state["heap"] = ESP.getFreeHeap();
 *         state["temp"] = temperatureRead();
 *     });
 *
 * _diag.addSensor("sensor", "status", "Status", "mdi:heart-pulse", HACategory::Diagnostic,
 *         [](JsonObject &c) { c["value_template"] = "{{value_json.state}}"; })
 *     .addSensor("sensor", "free_heap", "Free Heap", "mdi:memory", HACategory::Diagnostic,
 *         [](JsonObject &c) {
 *             c["value_template"]      = "{{value_json.heap}}";
 *             c["unit_of_measurement"] = "B";
 *         });
 *
 * _diag.begin();
 * // From a timer callback:
 * _diag.publishState();
 * @endcode
 */
class HAGroupedSensorPublisher
{
public:
    using ConfigBuilder = std::function<void(JsonObject &config)>;
    using StateReader = std::function<void(JsonObject &state)>;

    /**
     * @param haService     framework HAService
     * @param topicSuffix   suffix used in the shared state topic
     *                      (~/<topicSuffix>/state). Slugified is recommended.
     * @param stateReader   producer for the shared state JSON; invoked on
     *                      publishAll() and publishState().
     * @param ownerDevice   optional sub-device owner. When set, discovery configs
     *                      are published via the sub-device (attaches its identity
     *                      block and scopes topics under its base topic). Pass
     *                      nullptr (default) to publish against the main device.
     */
    HAGroupedSensorPublisher(HAService *haService,
                             const String &topicSuffix,
                             StateReader stateReader,
                             HADevice *ownerDevice = nullptr);

    virtual ~HAGroupedSensorPublisher();

    /**
     * Register a sensor entity that lives on the shared state topic.
     *
     * @param component    "sensor" or "binary_sensor"
     * @param objectId     unique entity object id (per device)
     * @param name         human-readable entity name shown in HA
     * @param icon         optional MDI icon string (e.g. "mdi:thermometer")
     * @param category     HA entity_category (default: Control = no category key)
     * @param extraConfig  optional lambda for fields not covered above
     *                     (value_template, unit_of_measurement, device_class, …);
     *                     state_topic and unique_id are pre-populated.
     */
    HAGroupedSensorPublisher &addSensor(const String &component,
                                        const String &objectId,
                                        const String &name,
                                        const String &icon = "",
                                        HACategory category = HACategory::Control,
                                        ConfigBuilder extraConfig = {});

    /** Register with HAService::onPublishAll. */
    void begin();

    /** Publish all sensor configs and the current shared state. */
    virtual void publishAll();

    /**
     * Publish only the shared state (no config). Call from timers or other
     * push points. No-op if HAService is not ready.
     */
    void publishState();

    /**
     * Remove all entities from HA by sending empty retained payloads to every
     * discovery config topic and the shared state topic. Mirrors HADevice::unpublishAll().
     * No-op if HAService is not ready.
     */
    void unpublishAll();

protected:
    struct Sensor
    {
        String component;
        String objectId;
        String name;
        String icon;
        HACategory category;
        ConfigBuilder extraConfig;
    };

    HAService *_haService;
    HADevice *_ownerDevice;
    String _topicSuffix;
    StateReader _stateReader;
    std::vector<Sensor> _sensors;

    // Liveness flag captured by the lambdas registered with HAService in begin().
    // Set to false in the destructor so post-destruction invocations bail out
    // instead of dereferencing freed memory — HAService never deregisters its
    // _publishCallbacks/_unpublishCallbacks, so the lambdas can outlive us.
    std::shared_ptr<bool> _alive;

    String _sharedStateTopicRel() const;
    String _sharedStateTopicAbs() const;

    String _deviceId() const;

    void _publishSensorConfig(const Sensor &sensor);
};
