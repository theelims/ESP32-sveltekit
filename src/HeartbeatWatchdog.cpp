/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <HeartbeatWatchdog.h>

const char *TAG = "HeartbeatWatchdog";

HeartbeatWatchdog::HeartbeatWatchdog(uint32_t interval) : _heartbeatInterval(interval),
                                                          _mode(WATCHDOG_MODE_NONE),
                                                          _onWatchdogCallbacks(),
                                                          _onClientMissingCallbacks()
{
    _parameterMutex = xSemaphoreCreateMutex();
    ESP_LOGV(TAG, "HeartbeatWatchdog created with interval %d ms", interval);
}

HeartbeatWatchdog::~HeartbeatWatchdog()
{
    setWatchdogMode(WATCHDOG_MODE_NONE);
    _onWatchdogCallbacks.clear();
    _onClientMissingCallbacks.clear();
    _clientHeartbeatMap.clear();
}

void HeartbeatWatchdog::setWatchdogMode(WatchdogMode mode)
{
    _mode = mode;

    if (_mode != WATCHDOG_MODE_NONE)
    {
        ESP_LOGI(TAG, "Enabling watchdog mode: %d", _mode);
        _clientHeartbeatMap.clear(); // clear all clients and start over fresh
        _startHealthCheck();
    }
    else
    {
        ESP_LOGI(TAG, "Disabling watchdog");
        if (_taskHealthCheckHandle != NULL)
        {
            vTaskDelete(_taskHealthCheckHandle);
            _taskHealthCheckHandle = NULL;
        }
    }
}

WatchdogMode HeartbeatWatchdog::getWatchdogMode()
{
    return _mode;
}

void HeartbeatWatchdog::setHeartbeatInterval(uint32_t interval)
{
    _heartbeatInterval = interval;
    ESP_LOGI(TAG, "Heartbeat interval set to %d ms", interval);
}

uint32_t HeartbeatWatchdog::getHeartbeatInterval()
{
    return _heartbeatInterval;
}

void HeartbeatWatchdog::addClient(String clientId)
{
    // acquire lock
    if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
    {
        _clientHeartbeatMap[clientId.c_str()] = millis();
        xSemaphoreGive(_parameterMutex);
        ESP_LOGI(TAG, "Added client [%s]", clientId.c_str());
    }
}

void HeartbeatWatchdog::removeClient(String clientId)
{
    // acquire lock
    if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
    {
        _clientHeartbeatMap.erase(clientId.c_str());
        xSemaphoreGive(_parameterMutex);
        ESP_LOGI(TAG, "Removed client [%s]", clientId.c_str());
    }
}

void HeartbeatWatchdog::heartbeat(String clientId)
{
    // acquire lock
    if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
    {
        _clientHeartbeatMap[clientId.c_str()] = millis();
        ESP_LOGV(TAG, "Heartbeat tick for client [%s]", clientId.c_str());
        xSemaphoreGive(_parameterMutex);
    }
}

void HeartbeatWatchdog::onWatchdog(onWatchdogCallback callback)
{
    _onWatchdogCallbacks.push_back(callback);
}

void HeartbeatWatchdog::onClientMissing(onClientMissingCallback callback)
{
    _onClientMissingCallbacks.push_back(callback);
}

uint32_t HeartbeatWatchdog::numberOfClients()
{
    return _clientHeartbeatMap.size();
}

bool HeartbeatWatchdog::isClientAlive(String clientId)
{
    // acquire lock
    if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
    {
        bool alive = _clientHeartbeatMap.find(clientId.c_str()) != _clientHeartbeatMap.end();
        xSemaphoreGive(_parameterMutex);
        ESP_LOGV(TAG, "Client [%s] is %s", clientId.c_str(), alive ? "alive" : "dead");
        return alive;
    }
    return false;
}

void HeartbeatWatchdog::_startHealthCheck()
{
    xTaskCreatePinnedToCore(_healthCheckImpl, "healthCheck", 4096, this, tskIDLE_PRIORITY + 1, &_taskHealthCheckHandle, 0);
}

void HeartbeatWatchdog::_missingClient(std::string clientId)
{
    _clientHeartbeatMap.erase(clientId);
    ESP_LOGW(TAG, "Client [%s] went missing. %s clients remaining", clientId.c_str(), numberOfClients());
    for (auto &callback : _onClientMissingCallbacks)
    {
        callback(String(clientId.c_str()));
    }

    // trigger watchdog if no clients are left or mode is set to any
    if ((!_mode == WATCHDOG_MODE_NONE && numberOfClients() == 0) || _mode == WATCHDOG_MODE_ANY)
    {
        ESP_LOGW(TAG, "Watchdog Alarm triggered");
        for (auto &callback : _onWatchdogCallbacks)
        {
            callback(String(clientId.c_str()));
        }
    }
}

void HeartbeatWatchdog::_healthCheck()
{
    while (1)
    {
        ESP_LOGV(TAG, "Health check running");
        // acquire lock
        if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
        {
            for (auto &client : _clientHeartbeatMap)
            {
                if (millis() - client.second > _heartbeatInterval)
                {
                    _missingClient(client.first);
                }
            }
            xSemaphoreGive(_parameterMutex);
        }
        vTaskDelay(_heartbeatInterval / portTICK_PERIOD_MS);
    }
}
