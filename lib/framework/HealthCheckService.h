#ifndef HealthCheckService_h
#define HealthCheckService_h

/**
 *   HealthCheckService - Extensible Health Check Endpoint
 *
 *   Provides a /rest/health endpoint with callback-based extensibility
 *   for the ESP32 SvelteKit framework.
 *
 *   Copyright (C) 2025 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <PsychicHttp.h>
#include <SecurityManager.h>
#include <ArduinoJson.h>
#include <functional>
#include <list>
#include <exception>

#define HEALTH_CHECK_SERVICE_PATH "/rest/health"

/**
 * HealthCheckService provides an extensible health check endpoint at /rest/health
 * 
 * Basic usage returns: {"up": true}
 * 
 * Extended usage example:
 * 
 *   // Register a health check callback that adds WiFi status
 *   esp32SvelteKit.getHealthCheckService()->addHealthCheckCallback([](JsonObject &json) {
 *       json["wifi"]["connected"] = WiFi.isConnected();
 *       json["wifi"]["rssi"] = WiFi.RSSI();
 *   });
 * 
 *   // Register a callback that adds memory info
 *   esp32SvelteKit.getHealthCheckService()->addHealthCheckCallback([](JsonObject &json) {
 *       json["memory"]["free"] = ESP.getFreeHeap();
 *       json["memory"]["total"] = ESP.getHeapSize();
 *   });
 * 
 * This would result in a response like:
 * {
 *   "up": true,
 *   "wifi": {"connected": true, "rssi": -45},
 *   "memory": {"free": 123456, "total": 327680}
 * }
 */

typedef size_t health_check_handler_id_t;
typedef std::function<void(JsonObject &json)> HealthCheckCallback;

typedef struct HealthCheckHandlerInfo
{
    static health_check_handler_id_t currentHandlerId;
    health_check_handler_id_t _id;
    HealthCheckCallback _cb;
    bool _allowRemove;
    HealthCheckHandlerInfo(HealthCheckCallback cb, bool allowRemove) : _id(++currentHandlerId), _cb(cb), _allowRemove(allowRemove) {};
} HealthCheckHandlerInfo_t;

class HealthCheckService
{
public:
    HealthCheckService(PsychicHttpServer *server, SecurityManager *securityManager);

    void begin();

    /**
     * @brief Add a health check callback that contributes to the JSON response
     * @param cb Callback function that receives a JsonObject reference to populate
     * @param allowRemove Whether this callback can be removed later (default: true)
     * @return Handler ID (>0) that can be used to remove the callback. Returns 0 if the callback is null.
     */
    health_check_handler_id_t addHealthCheckCallback(HealthCheckCallback cb, bool allowRemove = true);
    
    /**
     * @brief Remove a health check callback by its handler ID
     * @param id The handler ID returned by addHealthCheckCallback()
     */
    void removeHealthCheckCallback(health_check_handler_id_t id);

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    std::list<HealthCheckHandlerInfo_t> _healthCheckHandlers;
    
    esp_err_t healthCheck(PsychicRequest *request);
    void callHealthCheckHandlers(JsonObject &json);
};

#endif // end HealthCheckService_h