#pragma once
/**
 *   LUST-motion
 *
 *   https://github.com/openlust/LUST-motion
 *
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the Attribution-ShareAlike 4.0 International license.
 *   See the LICENSE file for details.
 *
 **/

#include <PsychicHttp.h>
#include <ESP32SvelteKit.h>
#include <EventSocket.h>
#include <StrokeEngine.h>

#ifndef DATA_STREAMING_INTERVAL
#define DATA_STREAMING_INTERVAL 50
#endif

#define RAW_POSITION_EVENT "data"
#define AGGREGATION 5

#define MOTOR_STATE_EVENT "motor"
#define MOTOR_STATE_INTERVAL 500

#define rdTAG "RawPositionStreamer"

class DataStreamer
{
public:
    DataStreamer(EventSocket *socket, StrokeEngine *stroker) : _socket(socket),
                                                               _stroker(stroker)
    {
    }

    void begin()
    {
        _socket->registerEvent(RAW_POSITION_EVENT);
        _socket->registerEvent(MOTOR_STATE_EVENT);

        _stroker->getMotor()->attachPositionFeedback(std::bind(&DataStreamer::aggregateMotorData,
                                                               this,
                                                               std::placeholders::_1,
                                                               std::placeholders::_2,
                                                               std::placeholders::_3,
                                                               std::placeholders::_4),
                                                     DATA_STREAMING_INTERVAL);

        _initData();
    }

    void aggregateMotorData(float position, float speed, float current, float voltage)
    {
        // JSON Array
        JsonArray dataPoint = data.add<JsonArray>();
        dataPoint.add(millis());
        dataPoint.add(position);
        dataPoint.add(speed);
        dataPoint.add(current);
        dataPoint.add(voltage);

        i++;
        if (i > AGGREGATION)
        {
            _sendPacket();
            i = 0;
        }

        // Send motor state notification events every 500ms
        _sendMotorState();
    }

private:
    EventSocket *_socket;
    StrokeEngine *_stroker;

    JsonDocument motorDoc;
    JsonDocument dataDoc;
    JsonArray data;

    unsigned int lastMillis = 0;

    int i = 0;

    void _sendMotorState()
    {
        if (millis() - lastMillis > MOTOR_STATE_INTERVAL)
        {
            motorDoc["homed"] = _stroker->getMotor()->isHomed();
            motorDoc["error"] = _stroker->getMotor()->hasError();
            JsonObject jsonObject = motorDoc.as<JsonObject>();
            _socket->emitEvent(MOTOR_STATE_EVENT, jsonObject);
            lastMillis = millis();
        }
    }

    void _sendPacket()
    {
        JsonObject jsonObject = dataDoc.as<JsonObject>();
        serializeJson(jsonObject, Serial);
        _socket->emitEvent(RAW_POSITION_EVENT, jsonObject);
        _initData();
    }

    void _initData()
    {
        dataDoc.clear();
        data = dataDoc.add<JsonArray>();
    }
};
