/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <CommitHandler.h>

// Static member initialization
uint32_t CommitHandler::_timerIntervalMs = 0; // Default: throttling disabled
SemaphoreHandle_t CommitHandler::_timerMutex = nullptr;

CommitHandler::CommitHandler() : _timer(nullptr), _pendingCommit(false)
{
    // Initialize the global timer mutex if not already done
    if (_timerMutex == nullptr)
    {
        _timerMutex = xSemaphoreCreateMutex();
    }
    
    // Create instance mutex for thread safety
    _instanceMutex = xSemaphoreCreateMutex();
}

CommitHandler::~CommitHandler()
{
    if (_timer != nullptr)
    {
        xTimerDelete(_timer, portMAX_DELAY);
        _timer = nullptr;
    }
    
    if (_instanceMutex != nullptr)
    {
        vSemaphoreDelete(_instanceMutex);
    }
}

void CommitHandler::publish()
{
    if (xSemaphoreTake(_instanceMutex, portMAX_DELAY) == pdTRUE)
    {
        uint32_t currentInterval = getTimerInterval();
        
        if (currentInterval == 0)
        {
            // Throttling disabled - commit immediately
            _pendingCommit = false;
            xSemaphoreGive(_instanceMutex);
            commit();
        }
        else
        {
            // Throttling enabled - schedule commit via timer
            _pendingCommit = true;
            startOrResetTimer();
            xSemaphoreGive(_instanceMutex);
        }
    }
}

void CommitHandler::setTimerInterval(uint32_t intervalMs)
{
    if (_timerMutex != nullptr && xSemaphoreTake(_timerMutex, portMAX_DELAY) == pdTRUE)
    {
        _timerIntervalMs = intervalMs;
        xSemaphoreGive(_timerMutex);
    }
}

uint32_t CommitHandler::getTimerInterval()
{
    uint32_t interval = 0;
    if (_timerMutex != nullptr && xSemaphoreTake(_timerMutex, portMAX_DELAY) == pdTRUE)
    {
        interval = _timerIntervalMs;
        xSemaphoreGive(_timerMutex);
    }
    return interval;
}

void CommitHandler::timerCallback(TimerHandle_t xTimer)
{
    CommitHandler* instance = static_cast<CommitHandler*>(pvTimerGetTimerID(xTimer));
    if (instance != nullptr)
    {
        instance->handleTimerExpired();
    }
}

void CommitHandler::handleTimerExpired()
{
    if (xSemaphoreTake(_instanceMutex, portMAX_DELAY) == pdTRUE)
    {
        if (_pendingCommit)
        {
            _pendingCommit = false;
            xSemaphoreGive(_instanceMutex);
            commit();
        }
        else
        {
            xSemaphoreGive(_instanceMutex);
        }
    }
}

void CommitHandler::startOrResetTimer()
{
    uint32_t currentInterval = getTimerInterval();
    
    if (currentInterval == 0)
    {
        // Timer disabled, stop existing timer if any
        stopTimer();
        return;
    }
    
    if (_timer == nullptr)
    {
        // Create the timer if it doesn't exist
        _timer = xTimerCreate(
            "CommitTimer",
            pdMS_TO_TICKS(currentInterval),
            pdFALSE, // One-shot timer
            this,    // Timer ID is pointer to this instance
            timerCallback
        );
    }
    else
    {
        // Update the timer period if it already exists
        xTimerChangePeriod(_timer, pdMS_TO_TICKS(currentInterval), 0);
    }
    
    if (_timer != nullptr)
    {
        // Start or reset the timer
        if (xTimerIsTimerActive(_timer))
        {
            xTimerReset(_timer, 0);
        }
        else
        {
            xTimerStart(_timer, 0);
        }
    }
}

void CommitHandler::stopTimer()
{
    if (_timer != nullptr && xTimerIsTimerActive(_timer))
    {
        xTimerStop(_timer, 0);
    }
}