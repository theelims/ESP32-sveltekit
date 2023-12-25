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

#include <WebSocketServer.h>
#include <ESP32SvelteKit.h>
#include "CBOR.h"
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#define RAW_POSITION_SOCKET_PATH "/ws/rawPosition"
#define CBORS_DEFAULT_ARRAY_SIZE 256
#define WS_PACKET_AGGREGATION_ARRAY 5

#define rdTAG "RawPositionStreaming"

class WebSocketRawDataStreamer
{
public:
    WebSocketRawDataStreamer(AsyncWebServer *server) : _server(server)
    {
        // add raw data websocket
        ws.onEvent(std::bind(&WebSocketRawDataStreamer::onWSEvent,
                             this,
                             std::placeholders::_1,
                             std::placeholders::_2,
                             std::placeholders::_3,
                             std::placeholders::_4,
                             std::placeholders::_5,
                             std::placeholders::_6));
        _server->addHandler(&ws);

        // prepare buffer
        rawDataWSPrint.reset();
        cborWS.resetWriteSize();
        cborWS.beginIndefiniteArray();
    }

    void streamRawData(unsigned int time, float position, float speed, float voltage, float current)
    {
        cborWS.beginArray(5);

        // write timestamp
        cborWS.writeUnsignedInt(time);
        cborWS.writeFloat(position);
        cborWS.writeFloat(speed);
        cborWS.writeFloat(voltage);
        cborWS.writeFloat(current);
        i++;

        if (i >= WS_PACKET_AGGREGATION_ARRAY - 1)
        {
            _sendPacket();
            i = 0;
        }
    }

private:
    AsyncWebServer *_server;
    AsyncWebSocket ws = AsyncWebSocket(RAW_POSITION_SOCKET_PATH);
    uint8_t rawDataWSBytes[CBORS_DEFAULT_ARRAY_SIZE]{0};
    qindesign::cbor::BytesPrint rawDataWSPrint{rawDataWSBytes, sizeof(rawDataWSBytes)};
    qindesign::cbor::Writer cborWS{rawDataWSPrint};
    int i = 0;

    void _sendPacket()
    {
        // end indefinite array
        cborWS.endIndefinite();

        // Send data over websocket
        size_t length = cborWS.getWriteSize();
        ws.binaryAll(reinterpret_cast<uint8_t *>(&rawDataWSBytes), length);

        // Prepare buffer for next chunk of data
        rawDataWSPrint.reset();
        cborWS.resetWriteSize();
        cborWS.beginIndefiniteArray();
    }

    void onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
        if (type == WS_EVT_CONNECT)
        {
            Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
            client->ping();
        }
        else if (type == WS_EVT_DISCONNECT)
        {
            Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
        }
        else if (type == WS_EVT_ERROR)
        {
            Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
        }
        else if (type == WS_EVT_PONG)
        {
            Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
        }
    }
};
