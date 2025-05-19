#pragma once

#include <ESP32SvelteKit.h>
#include <cc1101.h>

#define WEB_SOCKET_LOGGER_ORIGIN "wslogger"
#define WEB_SOCKET_LOGGER_ORIGIN_CLIENT_ID_PREFIX "wslogger:"
#define WEB_SOCKET_LOGGER_PATH "/ws/logger"

class WebSocketLogger
{
public:
WebSocketLogger(ESP32SvelteKit *sveltekit) : _server(sveltekit->getServer()),
                                             _securityManager(sveltekit->getSecurityManager())
    {
    }

    void begin()
    {
        _webSocket.setFilter(_securityManager->filterRequest(AuthenticationPredicates::NONE_REQUIRED));
        _webSocket.onOpen(std::bind(&WebSocketLogger::onWSOpen,
                                    this,
                                    std::placeholders::_1));
        _webSocket.onClose(std::bind(&WebSocketLogger::onWSClose,
                                     this,
                                     std::placeholders::_1));
        _webSocket.onFrame(std::bind(&WebSocketLogger::onWSFrame,
                                     this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
        _server->on(WEB_SOCKET_LOGGER_PATH, &_webSocket);

        ESP_LOGV(WebSocketLogger::TAG, "Registered WebSocket handler: %s", WEB_SOCKET_LOGGER_PATH);
    }

    void onWSOpen(PsychicWebSocketClient *client)
    {
        // when a client connects, we transmit it's id and the current payload
        transmitId(client);

        ESP_LOGI(WebSocketLogger::TAG, "ws[%s][%u] connect", client->remoteIP().toString().c_str(), client->socket());
        ESP_LOGV(WebSocketLogger::TAG, "Number of connected clients: %d", _webSocket.count());
    }

    void onWSClose(PsychicWebSocketClient *client)
    {
        ESP_LOGI(WebSocketLogger::TAG, "ws[%s][%u] disconnect", client->remoteIP().toString().c_str(), client->socket());
    }

    esp_err_t onWSFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame)
    {
        ESP_LOGV(WebSocketLogger::TAG, "ws[%s][%u] opcode[%d]", request->client()->remoteIP().toString().c_str(), request->client()->socket(), frame->type);

        if (frame->type == HTTPD_WS_TYPE_BINARY)
        {
            ESP_LOGV(WebSocketLogger::TAG, "ws[%s][%u] request: %s", request->client()->remoteIP().toString().c_str(), request->client()->socket(), (char *)frame->payload);
        }
        return ESP_OK;
    }

    String clientId(PsychicWebSocketClient *client)
    {
        return WEB_SOCKET_LOGGER_ORIGIN_CLIENT_ID_PREFIX + String(client->socket());
    }

    void logPacket(cc1101_packet_t *packet)
    {
        //_webSocket.sendAll(HTTPD_WS_TYPE_BINARY, packet->data, packet->length);
        _webSocket.sendAll(HTTPD_WS_TYPE_BINARY, packet, sizeof(cc1101_packet_t));
    }

    void startPeriodicPacketTransmission()
    {
        static TaskHandle_t taskHandle = nullptr;

        if (taskHandle == nullptr)
        {
            xTaskCreatePinnedToCore(
                [](void *param) {
                    WebSocketLogger *logger = static_cast<WebSocketLogger *>(param);
                    while (true)
                    {
                        cc1101_packet_t packet;
                        // Populate the packet with data (example data here)
                        memset(&packet, 0, sizeof(packet));
                        packet.length = 10; // Example length
                        for (int i = 0; i < packet.length; ++i)
                        {
                            packet.data[i] = i; // Example payload
                        }

                        logger->logPacket(&packet);
                        vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 ms
                    }
                },
                "PeriodicPacketTask",
                4096, // Stack size
                this,
                1, // Priority
                &taskHandle,
                APP_CPU_NUM // Run on application CPU
            );
        }
    }

private:
    static constexpr const char *TAG = "WebSocketLogger";

    SecurityManager *_securityManager;
    PsychicHttpServer *_server;
    PsychicWebSocketHandler _webSocket;

    void transmitId(PsychicWebSocketClient *client)
    {
        JsonDocument jsonDocument;
        JsonObject root = jsonDocument.to<JsonObject>();
        root["type"] = "id";
        root["id"] = clientId(client);

        // serialize the json to a string
        String buffer;
        serializeJson(jsonDocument, buffer);
        client->sendMessage(buffer.c_str());
    }
};

