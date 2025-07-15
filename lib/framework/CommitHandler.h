#ifndef CommitHandler_h
#define CommitHandler_h

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

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/semphr.h>

/**
 * CommitHandler provides a throttling mechanism for publish operations.
 * When timer interval > 0, publish calls are throttled and only committed after the timer expires.
 * When timer interval = 0, publish calls are committed immediately (throttling disabled).
 */
class CommitHandler
{
public:
    CommitHandler();
    virtual ~CommitHandler();

    /**
     * Request a publish operation. If throttling is enabled, this will schedule a commit.
     * If throttling is disabled (timer interval = 0), this will commit immediately.
     */
    void publish();

    /**
     * Set the global timer interval for all CommitHandler instances.
     * @param intervalMs Timer interval in milliseconds. 0 disables throttling.
     */
    static void setTimerInterval(uint32_t intervalMs);

    /**
     * Get the current global timer interval.
     * @return Timer interval in milliseconds. 0 means throttling is disabled.
     */
    static uint32_t getTimerInterval();

protected:
    /**
     * This method must be implemented by derived classes to perform the actual commit operation.
     * It will be called either immediately (if throttling disabled) or after the timer expires.
     */
    virtual void commit() = 0;

private:
    static uint32_t _timerIntervalMs;
    static SemaphoreHandle_t _timerMutex;
    
    TimerHandle_t _timer;
    SemaphoreHandle_t _instanceMutex;
    bool _pendingCommit;

    static void timerCallback(TimerHandle_t xTimer);
    void handleTimerExpired();
    void startOrResetTimer();
    void stopTimer();
};

#endif // end CommitHandler_h