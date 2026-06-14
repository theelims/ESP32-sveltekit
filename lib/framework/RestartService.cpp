/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *   Copyright (C) 2026 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <RestartService.h>

// Static member initialization
PsychicHttpServer *RestartService::_server = nullptr;
#if FT_ENABLED(FT_MQTT)
MqttSettingsService *RestartService::_mqttSettingsService = nullptr;
#endif

#if FT_ENABLED(FT_MQTT)
RestartService::RestartService(PsychicHttpServer *server, SecurityManager *securityManager, MqttSettingsService *mqttSettingsService)
    : _securityManager(securityManager)
{
    _server = server;
    _mqttSettingsService = mqttSettingsService;
}
#else
RestartService::RestartService(PsychicHttpServer *server, SecurityManager *securityManager)
    : _securityManager(securityManager)
{
    _server = server;
}
#endif

void RestartService::begin()
{
    _server->on(RESTART_SERVICE_PATH,
                HTTP_POST,
                _securityManager->wrapRequest(std::bind(&RestartService::restart, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_ADMIN));

    ESP_LOGV(SVK_TAG, "Registered POST endpoint: %s", RESTART_SERVICE_PATH);
}

void RestartService::restartNow()
{
    ESP_LOGI("RestartService", "Device restart scheduled");

    // Spawn a one-shot task to perform the restart sequence.
    // This MUST run in a separate task because httpd_stop() sends a
    // shutdown message to the httpd thread and then blocks waiting for
    // it to finish. If restartNow() is called from within a request
    // handler (which executes ON the httpd thread), httpd_stop() would
    // deadlock — the handler waits for httpd to stop, but httpd can't
    // stop because the handler hasn't returned yet.
    xTaskCreate(_restartTask, "restart", 4096, nullptr, 1, nullptr);
}

void RestartService::_restartTask(void *param)
{
    // Let the calling handler return so the httpd thread is free to
    // process the shutdown message.
    vTaskDelay(pdMS_TO_TICKS(200));

#if FT_ENABLED(FT_MQTT)
    // Gracefully disconnect the MQTT client before killing WiFi.
    // MqttSettingsService::disconnect() publishes "offline" synchronously,
    // sends the MQTT DISCONNECT packet and stops the MQTT task.
    if (_mqttSettingsService)
    {
        ESP_LOGI("RestartService", "Disconnecting MQTT client...");
        _mqttSettingsService->shutdown();
        ESP_LOGI("RestartService", "MQTT client disconnected.");
    }
#endif

    // Gracefully stop the HTTP server.
    // httpd_stop() → httpd_sess_close_all() → httpd_sess_delete() per session:
    //   - With SO_LINGER enabled (2s), close() blocks until pcb->unsent and
    //     pcb->unacked are empty, i.e. the TCP stack has transmitted all data
    //     AND received ACKs from the client — or the timeout expires.
    //   - This ensures WebSocket events and HTTP responses are fully delivered.
    if (_server)
    {
        ESP_LOGI("RestartService", "Stopping HTTP server...");
        _server->stop();
        ESP_LOGI("RestartService", "HTTP server stopped.");
    }

    MDNS.end();
    WiFi.disconnect(true);
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP.restart();
}

esp_err_t RestartService::restart(PsychicRequest *request)
{
    request->reply(200);
    restartNow();
    return ESP_OK;
}
