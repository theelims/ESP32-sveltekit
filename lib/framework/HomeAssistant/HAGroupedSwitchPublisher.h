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
#include <StatefulService.h>
#include <HomeAssistant/HAService.h>
#include <HomeAssistant/HAEntityBase.h>

/**
 * @brief Multi-switch publisher sharing one MQTT state topic, with per-switch
 *        command topics. Bidirectional counterpart to HAGroupedSensorPublisher.
 *
 * A group of HA switch entities is bound to N boolean fields of a single
 * StatefulService<T>. The shared state topic — `~/<topicSuffix>/state` —
 * carries a JSON object with all switch states, each entity selects its
 * value via `value_template`. Commands arrive on dedicated per-switch topics
 * (`~/<topicSuffix>/<objectId>/set`) so HA can address each toggle
 * individually.
 *
 * Pull-based: state JSON is rebuilt from the StatefulService<T> via the
 * registered getters whenever publishState() runs. State is republished on
 * every state change of the StatefulService<T> (so HA stays in sync after
 * any local/HTTP/Event edit) and on every MQTT (re)connect.
 *
 * Usage:
 * @code
 * struct DemoSettings { bool a; bool b; };
 *
 * HAGroupedSwitchPublisher<DemoSettings> _switches(this, ha, "config");
 *
 * _switches.addSwitch("feature_a", "Feature A",
 *     [](const DemoSettings &s) { return s.a; },
 *     [](DemoSettings &s, bool v) { s.a = v; },
 *     "mdi:toggle-switch", HACategory::Config);
 *
 * _switches.addSwitch("feature_b", "Feature B",
 *     [](const DemoSettings &s) { return s.b; },
 *     [](DemoSettings &s, bool v) { s.b = v; });
 *
 * _switches.begin();
 * @endcode
 */
template <class T>
class HAGroupedSwitchPublisher
{
public:
    using ConfigBuilder = std::function<void(JsonObject &config)>;
    using Getter        = std::function<bool(const T &)>;
    using Setter        = std::function<void(T &, bool)>;

    /**
     * @param statefulService  StatefulService<T> holding the bool fields
     * @param haService        framework HAService
     * @param topicSuffix      suffix for the shared state topic
     *                         (~/<topicSuffix>/state). Slugified is recommended.
     */
    HAGroupedSwitchPublisher(StatefulService<T> *statefulService,
                             HAService *haService,
                             const String &topicSuffix)
        : _statefulService(statefulService),
          _haService(haService),
          _topicSuffix(topicSuffix),
          _alive(std::make_shared<bool>(true))
    {
        if (_statefulService != nullptr)
        {
            _statefulService->addUpdateHandler(
                [this](const String &originId)
                {
                    publishState();
                },
                false);
        }
    }

    virtual ~HAGroupedSwitchPublisher()
    {
        *_alive = false;
    }

    /**
     * Register a switch entity bound to one bool field of T.
     *
     * @param objectId     unique entity object id (per device)
     * @param name         human-readable entity name shown in HA
     * @param getter       reads the field's current bool value from T
     * @param setter       writes a new bool value into T
     * @param icon         optional MDI icon string (e.g. "mdi:brightness-6")
     * @param category     HA entity_category (default: Control = no category key)
     * @param extraConfig  optional escape-hatch lambda for fields not covered
     *                     above; runs after the typed fields are applied
     */
    HAGroupedSwitchPublisher &addSwitch(const String &objectId,
                                        const String &name,
                                        Getter getter,
                                        Setter setter,
                                        const String &icon = "",
                                        HACategory category = HACategory::Control,
                                        ConfigBuilder extraConfig = {})
    {
        _switches.push_back({objectId, name, icon, category, extraConfig, getter, setter});
        return *this;
    }

    /** Register with HAService::onPublishAll and onUnpublishAll. */
    void begin()
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

    /** Publish all switch configs, the current shared state, and subscribe. */
    virtual void publishAll()
    {
        if (_haService == nullptr || !_haService->isReady())
            return;

        for (const auto &sw : _switches)
        {
            _publishSwitchConfig(sw);
            _subscribe(sw);
        }

        publishState();
    }

    /** Remove all switch entities from HA. No-op if HAService is not ready. */
    void unpublishAll()
    {
        if (_haService == nullptr || !_haService->isReady())
            return;

        _haService->publish(_sharedStateTopicAbs(), String(), 0, true, false);

        for (const auto &sw : _switches)
        {
            String configTopic = _haService->getDiscoveryPrefix() + "switch" +
                                 "/" + _haService->getDeviceId() + "/" + sw.objectId + "/config";
            _haService->publish(configTopic, String(), 0, true, false);
        }
    }

    /** Publish only the shared state JSON. No-op if HAService is not ready. */
    void publishState()
    {
        if (_haService == nullptr || !_haService->isReady() || _statefulService == nullptr)
            return;

        JsonDocument doc;
        JsonObject root = doc.to<JsonObject>();

        _statefulService->read([&](T &s)
                               {
            for (const auto &sw : _switches)
            {
                if (sw.getter)
                    root[sw.objectId] = sw.getter(s) ? "ON" : "OFF";
            } });

        String payload;
        serializeJson(doc, payload);
        _haService->publish(_sharedStateTopicAbs(), payload);
    }

protected:
    struct Switch
    {
        String objectId;
        String name;
        String icon;
        HACategory category;
        ConfigBuilder extraConfig;
        Getter getter;
        Setter setter;
    };

    StatefulService<T> *_statefulService;
    HAService *_haService;
    String _topicSuffix;
    std::vector<Switch> _switches;

    // Liveness flag captured by the lambdas registered with HAService in begin().
    // Set to false in the destructor so post-destruction invocations bail out
    // instead of dereferencing freed memory — HAService never deregisters its
    // _publishCallbacks/_unpublishCallbacks, so the lambdas can outlive us.
    std::shared_ptr<bool> _alive;

    String _sharedStateTopicRel() const
    {
        return "~/" + _topicSuffix + "/state";
    }

    String _sharedStateTopicAbs() const
    {
        return _haService->getBaseTopic() + "/" + _topicSuffix + "/state";
    }

    String _commandTopicRel(const String &objectId) const
    {
        return "~/" + _topicSuffix + "/" + objectId + "/set";
    }

    String _commandTopicAbs(const String &objectId) const
    {
        return _haService->getBaseTopic() + "/" + _topicSuffix + "/" + objectId + "/set";
    }

    void _publishSwitchConfig(const Switch &sw)
    {
        JsonDocument config;
        config["state_topic"]    = _sharedStateTopicRel();
        config["command_topic"]  = _commandTopicRel(sw.objectId);
        config["value_template"] = "{{ value_json." + sw.objectId + " }}";
        config["unique_id"]      = _haService->getDeviceId() + "_" + sw.objectId;

        if (!sw.name.isEmpty())
            config["name"] = sw.name;

        if (!sw.icon.isEmpty())
            config["icon"] = sw.icon;

        switch (sw.category)
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

        if (sw.extraConfig)
        {
            JsonObject obj = config.as<JsonObject>();
            sw.extraConfig(obj);
        }

        _haService->publishConfig("switch", sw.objectId, config);
    }

    void _subscribe(const Switch &sw)
    {
        if (_haService == nullptr)
            return;

        Getter getter = sw.getter;
        Setter setter = sw.setter;
        _haService->subscribe(_commandTopicAbs(sw.objectId),
                              [this, getter, setter](char *topic, char *payload, int retain, int qos, bool dup)
                              {
                                  _onCommand(getter, setter, payload);
                              });
    }

    void _onCommand(Getter getter, Setter setter, const char *payload)
    {
        if (payload == nullptr || _statefulService == nullptr || !getter || !setter)
            return;

        bool requested;
        if (strcmp(payload, "ON") == 0)
            requested = true;
        else if (strcmp(payload, "OFF") == 0)
            requested = false;
        else
            return;

        _statefulService->update(
            [requested, getter, setter](T &s) -> StateUpdateResult
            {
                if (getter(s) == requested)
                    return StateUpdateResult::UNCHANGED;
                setter(s, requested);
                return StateUpdateResult::CHANGED;
            },
            HA_ORIGIN_ID);
    }
};
