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
#include "CBOR.h"
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#define RAW_POSITION_SOCKET_PATH "/ws/rawPosition"
#define CBORS_DEFAULT_ARRAY_SIZE 256
#define WS_PACKET_AGGREGATION_ARRAY 5

#define rdTAG "RawPositionStreamer"
class WebSocketRawDataStreamer
{
public:
    WebSocketRawDataStreamer(PsychicHttpServer *server) : _server(server)
    {
    }

    void begin()
    {
        _webSocket.onOpen(std::bind(&WebSocketRawDataStreamer::onWSOpen,
                                    this,
                                    std::placeholders::_1));
        _webSocket.onClose(std::bind(&WebSocketRawDataStreamer::onWSClose,
                                     this,
                                     std::placeholders::_1));
        _webSocket.onFrame(std::bind(&WebSocketRawDataStreamer::onWSFrame,
                                     this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
        _server->on(RAW_POSITION_SOCKET_PATH, &_webSocket);

        ESP_LOGV(rdTAG, "Registered WebSocket handler: %s", _webSocketPath.c_str());

        // prepare buffer
        rawDataWSPrint.reset();
        cborWS.resetWriteSize();
        cborWS.beginIndefiniteArray();
    }

    void streamRawData(unsigned int time, float position, float speed, float valueA, float valueB)
    {
        cborWS.beginArray(5);

        // write timestamp
        cborWS.writeUnsignedInt(time);
        cborWS.writeFloat(position);
        cborWS.writeFloat(speed);
        cborWS.writeFloat(valueA);
        cborWS.writeFloat(valueB);
        i++;

        if (i >= WS_PACKET_AGGREGATION_ARRAY - 1)
        {
            _sendPacket();
            i = 0;
        }
    }

private:
    PsychicHttpServer *_server;
    PsychicWebSocketHandler _webSocket;
    String _webSocketPath;
    size_t _bufferSize;
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
        _webSocket.sendAll(HTTPD_WS_TYPE_BINARY, reinterpret_cast<uint8_t *>(&rawDataWSBytes), length);

        // Prepare buffer for next chunk of data
        rawDataWSPrint.reset();
        cborWS.resetWriteSize();
        cborWS.beginIndefiniteArray();
    }

    void onWSOpen(PsychicWebSocketClient *client)
    {
        ESP_LOGI("WebSocketStreamer", "ws[%s][%u] connect", client->remoteIP().toString(), client->socket());
    }

    void onWSClose(PsychicWebSocketClient *client)
    {
        ESP_LOGI("WebSocketStreamer", "ws[%s][%u] disconnect", client->remoteIP().toString(), client->socket());
    }

    esp_err_t onWSFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame)
    {
        ESP_LOGV("WebSocketStreamer", "ws[%s][%u] opcode[%d]", request->client()->remoteIP().toString(), request->client()->socket(), frame->type);
        return ESP_OK;
    }
};
