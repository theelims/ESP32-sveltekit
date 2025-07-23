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

#define RAW_POSITION_EVENT "rawdata"
#define AGGREGATION 5

#define MOTOR_STATE_EVENT "motor"
#define MOTOR_STATE_INTERVAL 500

#define rdTAG "RawPositionStreamer"

class DataStreamer
{
public:
    DataStreamer(ESP32SvelteKit *esp32sveltekit, StrokeEngine *stroker);

    void begin();

    void aggregateMotorData(unsigned int time, float position, float speed, float current, float voltage);

private:
    EventSocket *_socket;
    StrokeEngine *_stroker;

    JsonDocument dataDoc;
    JsonObject root;
    JsonArray data;

    JsonDocument motorDoc;
    JsonObject motorRoot = motorDoc.to<JsonObject>();

    unsigned int lastMillis = 0;

    int i = 0;

    void _sendMotorState();

    void _sendPacket();

    void _initData();
};
