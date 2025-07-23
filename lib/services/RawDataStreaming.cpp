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

#include <RawDataStreaming.h>

DataStreamer::DataStreamer(ESP32SvelteKit *esp32sveltekit, StrokeEngine *stroker) : _socket(esp32sveltekit->getSocket()),
                                                                                    _stroker(stroker)
{
}

void DataStreamer::begin()
{
    _socket->registerEvent(RAW_POSITION_EVENT);
    _socket->registerEvent(MOTOR_STATE_EVENT);

    _stroker->getMotor()->attachPositionFeedback(std::bind(&DataStreamer::aggregateMotorData,
                                                           this,
                                                           std::placeholders::_1,
                                                           std::placeholders::_2,
                                                           std::placeholders::_3,
                                                           std::placeholders::_4,
                                                           std::placeholders::_5),
                                                 DATA_STREAMING_INTERVAL);

    _initData();
}

void DataStreamer::aggregateMotorData(unsigned int time, float position, float speed, float current, float voltage)
{
    // JSON Array
    JsonArray dataPoint = data.add<JsonArray>();
    dataPoint.add(time);
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

void DataStreamer::_sendMotorState()
{
    if (millis() - lastMillis > MOTOR_STATE_INTERVAL)
    {
        motorRoot["homed"] = _stroker->getMotor()->isHomed();
        motorRoot["error"] = _stroker->getMotor()->hasError();
        _socket->emitEvent(MOTOR_STATE_EVENT, motorRoot);
        lastMillis = millis();
    }
}

void DataStreamer::_sendPacket()
{
    _socket->emitEvent(RAW_POSITION_EVENT, root);
    _initData();
}

void DataStreamer::_initData()
{
    dataDoc.clear();
    root = dataDoc.to<JsonObject>();
    data = root["rawdata"].to<JsonArray>();
}
