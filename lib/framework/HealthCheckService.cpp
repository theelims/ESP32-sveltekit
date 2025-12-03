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

#include <HealthCheckService.h>
#include <SecurityManager.h>

// Initialize static member
health_check_handler_id_t HealthCheckHandlerInfo::currentHandlerId = 0;

HealthCheckService::HealthCheckService(PsychicHttpServer *server, SecurityManager *securityManager) : _server(server), _securityManager(securityManager)
{
}

void HealthCheckService::begin()
{
    _server->on(HEALTH_CHECK_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&HealthCheckService::healthCheck, this, std::placeholders::_1),
                                              AuthenticationPredicates::NONE_REQUIRED));

    ESP_LOGV(SVK_TAG, "Registered GET endpoint: %s", HEALTH_CHECK_SERVICE_PATH);
}

esp_err_t HealthCheckService::healthCheck(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject root = response.getRoot();
    
    // Always add the base "up" status
    root["up"] = true;
    
    // Call all registered health check handlers to contribute to the JSON
    callHealthCheckHandlers(root);
    
    return response.send();
}

health_check_handler_id_t HealthCheckService::addHealthCheckCallback(HealthCheckCallback cb, bool allowRemove)
{
    if (!cb)
    {
        ESP_LOGW(SVK_TAG, "Cannot add health check callback 'null'");
        return 0;
    }

    HealthCheckHandlerInfo_t handler(cb, allowRemove);
    _healthCheckHandlers.push_back(handler);
    return handler._id;
}

void HealthCheckService::removeHealthCheckCallback(health_check_handler_id_t id)
{
    if (id == 0)
    {
        ESP_LOGW(SVK_TAG, "Invalid health check handler ID '0' for removal");
        return;
    }

    for (auto i = _healthCheckHandlers.begin(); i != _healthCheckHandlers.end();)
    {
        if ((*i)._allowRemove && (*i)._id == id)
        {
            i = _healthCheckHandlers.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void HealthCheckService::callHealthCheckHandlers(JsonObject &json)
{
    for (const auto &handler : _healthCheckHandlers)
    {
        if (handler._cb)
        {
            try
            {
                handler._cb(json);
            }
            catch (const std::exception &e)
            {
                ESP_LOGW(SVK_TAG, "Health check callback failed: %s", e.what());
            }
            catch (...)
            {
                ESP_LOGW(SVK_TAG, "Health check callback failed with unknown exception");
            }
        }
    }
}