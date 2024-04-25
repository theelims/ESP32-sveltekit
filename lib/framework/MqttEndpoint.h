#ifndef MqttEndpoint_h
#define MqttEndpoint_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <StatefulService.h>
#include <PsychicMqttClient.h>

#define MQTT_ORIGIN_ID "mqtt"

template <class T>
class MqttEndpoint
{
public:
    MqttEndpoint(JsonStateReader<T> stateReader,
                 JsonStateUpdater<T> stateUpdater,
                 StatefulService<T> *statefulService,
                 PsychicMqttClient *mqttClient,
                 const String &pubTopic = "",
                 const String &subTopic = "",
                 bool retain = false) : _stateReader(stateReader),
                                        _stateUpdater(stateUpdater),
                                        _statefulService(statefulService),
                                        _mqttClient(mqttClient),
                                        _pubTopic(pubTopic),
                                        _subTopic(subTopic),
                                        _retain(retain)

    {
        _statefulService->addUpdateHandler([&](const String &originId)
                                           { publish(); },
                                           false);

        _mqttClient->onConnect(std::bind(&MqttEndpoint::onConnect, this));

        _mqttClient->onMessage(std::bind(&MqttEndpoint::onMqttMessage,
                                         this,
                                         std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3,
                                         std::placeholders::_4,
                                         std::placeholders::_5));
    }

public:
    void configureTopics(const String &pubTopic, const String &subTopic)
    {
        setSubTopic(subTopic);
        setPubTopic(pubTopic);
    }

    void setSubTopic(const String &subTopic)
    {
        if (!_subTopic.equals(subTopic))
        {
            // unsubscribe from the existing topic if one was set
            if (_subTopic.length() > 0)
            {
                _mqttClient->unsubscribe(_subTopic.c_str());
            }
            // set the new topic and re-configure the subscription
            _subTopic = subTopic;
            subscribe();
        }
    }

    void setPubTopic(const String &pubTopic)
    {
        _pubTopic = pubTopic;
        publish();
    }

    void setRetain(const bool retain)
    {
        _retain = retain;
        publish();
    }

    void publish()
    {
        if (_pubTopic.length() > 0 && _mqttClient->connected())
        {
            // serialize to json doc
            JsonDocument json;
            JsonObject jsonObject = json.to<JsonObject>();
            _statefulService->read(jsonObject, _stateReader);

            // serialize to string
            String payload;
            serializeJson(json, payload);

            // publish the payload
            _mqttClient->publish(_pubTopic.c_str(), 0, _retain, payload.c_str());
        }
    }

    PsychicMqttClient *getMqttClient()
    {
        return _mqttClient;
    }

protected:
    StatefulService<T> *_statefulService;
    PsychicMqttClient *_mqttClient;
    JsonStateUpdater<T> _stateUpdater;
    JsonStateReader<T> _stateReader;
    String _subTopic;
    String _pubTopic;
    bool _retain;

    void onMqttMessage(char *topic,
                       char *payload,
                       int retain,
                       int qos,
                       bool dup)
    {
        // we only care about the topic we are watching in this class
        if (strcmp(_subTopic.c_str(), topic))
        {
            return;
        }

        // deserialize from string
        JsonDocument json;
        DeserializationError error = deserializeJson(json, payload);
        if (!error && json.is<JsonObject>())
        {
            JsonObject jsonObject = json.as<JsonObject>();
            _statefulService->update(jsonObject, _stateUpdater, MQTT_ORIGIN_ID);
        }
    }

    void onConnect()
    {
        subscribe();
        publish();
    }

    void subscribe()
    {
        if (_subTopic.length() > 0)
        {
            _mqttClient->subscribe(_subTopic.c_str(), 2);
        }
    }
};

#endif // end MqttEndpoint
