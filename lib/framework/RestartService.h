#ifndef RestartService_h
#define RestartService_h

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

#include <WiFi.h>

#include <ESPmDNS.h>
#include <Features.h>
#include <PsychicHttp.h>
#include <SecurityManager.h>

#if FT_ENABLED(FT_MQTT)
#include <MqttSettingsService.h>
#endif

#define RESTART_SERVICE_PATH "/rest/restart"

class RestartService
{
public:
#if FT_ENABLED(FT_MQTT)
    RestartService(PsychicHttpServer *server, SecurityManager *securityManager, MqttSettingsService *mqttSettingsService);
#else
    RestartService(PsychicHttpServer *server, SecurityManager *securityManager);
#endif

    void begin();

    /**
     * @brief Schedules a graceful shutdown and restart of the ESP.
     *
     * Spawns a one-shot FreeRTOS task that performs the shutdown sequence
     * asynchronously. This is necessary because httpd_stop() waits for
     * the httpd thread to finish, and calling it from within a request
     * handler (which runs ON the httpd thread) would deadlock.
     *
     * The shutdown sequence (runs in a separate task):
     * 1. Brief delay to let the calling handler return to httpd.
     * 2. Gracefully disconnects the MQTT client: publishes "offline"
     *    status synchronously, sends DISCONNECT, stops the MQTT task.
     * 3. Stops the HTTP server (httpd_stop), which closes all sessions.
     *    With SO_LINGER enabled, each close() blocks until the TCP send
     *    buffer is flushed or the linger timeout (2s) expires.
     * 4. Stops mDNS.
     * 5. Disconnects WiFi and turns off the radio.
     * 6. Restarts the ESP.
     */
    static void restartNow();

private:
    static PsychicHttpServer *_server;
    SecurityManager *_securityManager;

#if FT_ENABLED(FT_MQTT)
    static MqttSettingsService *_mqttSettingsService;
#endif

    static void _restartTask(void *param);
    esp_err_t restart(PsychicRequest *request);
};

#endif // end RestartService_h
