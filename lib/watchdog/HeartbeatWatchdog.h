/**
 *   LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#ifndef HEARTBEAT_WATCHDOG_H
#define HEARTBEAT_WATCHDOG_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include <unordered_map>
#include <string>

// callback function type
typedef std::function<void(String clientId)> onWatchdogCallback;
typedef std::function<void(String clientId)> onClientMissingCallback;

/**
 * @brief Enumeration representing different watchdog modes.
 */
typedef enum
{
    /**
     * Watchdog mode indicating no watchdog functionality.
     */
    WATCHDOG_MODE_NONE = 0,

    /**
     * Watchdog mode indicating that any client who went missing triggers
     * the watchdog alarm.
     */
    WATCHDOG_MODE_ANY = 1,

    /**
     * Watchdog mode indicating the last available client disappearing
     * triggers the watchdog alarm.
     */
    WATCHDOG_MODE_LAST = 2
} WatchdogMode;

/**
 * @brief Class for monitoring the heartbeat of several clients. Each client
 * is expected to send a heartbeat at regular intervals. The class can
 * be configured to trigger a watchdog alarm when any of the clients goes missing,
 * or if the last available client goes missing. It can be disabled by setting
 * the watchdog mode to none. By actively removing clients from the list of
 * clients to monitor, the watchdog alarm can be avoided for that client. Unless
 * this was the last client being monitored, in which case the watchdog alarm
 * will be triggered nonetheless.
 */
class HeartbeatWatchdog
{
public:
    /**
     * @brief Construct a new HeartbeatWatchdog object.
     *
     * @param interval The interval in milliseconds between each check of the clients.
     *                Default is 1000ms.
     */
    HeartbeatWatchdog(uint32_t interval = 1000);

    /**
     * @brief Destroy the HeartbeatWatchdog object.
     */
    ~HeartbeatWatchdog();

    /**
     * @brief Set the watchdog mode.
     *
     * @param mode The watchdog mode to set.
     */
    void setWatchdogMode(WatchdogMode mode);

    /**
     * @brief Get the watchdog mode.
     *
     * @return The watchdog mode.
     */
    WatchdogMode getWatchdogMode();

    /**
     * @brief Set the interval between each check of the clients.
     *
     * @param interval The interval in milliseconds.
     */
    void setHeartbeatInterval(uint32_t interval);

    /**
     * @brief Get the interval between each check of the clients.
     *
     * @return The interval in milliseconds.
     */
    uint32_t getHeartbeatInterval();

    /**
     * @brief Add a client to the list of clients to monitor. ClientID
     *        is used as the unique identifier for the client.
     *
     * @param clientId The ID of the client to add.
     */
    void addClient(String clientId);

    /**
     * @brief Safely remove a client from the list of clients to monitor
     *        without triggering the watchdog alarm. If this was the last
     *        client being monitored, the watchdog alarm will be triggered
     *        nonetheless.
     *
     * @param clientId The ID of the client to remove.
     */
    void removeClient(String clientId);

    /**
     * @brief Trigger a heartbeat tick for the client with the given ID.
     *        New clients will be implicitly added to the list of clients.
     *
     * @param clientId The ID of the client to trigger a heartbeat for.
     */
    void heartbeat(String clientId);

    /**
     * @brief Add a callback function to be called when the watchdog is triggered.
     *
     * @param callback The callback function to add.
     */
    void onWatchdog(onWatchdogCallback callback);

    /**
     * @brief Add a callback function to be called when a client goes missing.
     *
     * @param callback The callback function to add.
     */
    void onClientMissing(onClientMissingCallback callback);

    /**
     * @brief Get the number of clients currently being monitored.
     *
     * @return The number of clients.
     */
    u_int32_t numberOfClients();

    /**
     * @brief Check if a client is alive.
     *
     * @param clientId The ID of the client to check.
     * @return true if the client is alive, false otherwise.
     */
    bool isClientAlive(String clientId);

private:
    uint32_t _heartbeatInterval;
    WatchdogMode _mode;

    std::vector<onWatchdogCallback> _onWatchdogCallbacks;
    std::vector<onClientMissingCallback> _onClientMissingCallbacks;

    std::unordered_map<std::string, uint32_t> _clientHeartbeatMap;

    void _startHealthCheck();
    void _missingClient(std::string clientId);

    static void _healthCheckImpl(void *_this) { static_cast<HeartbeatWatchdog *>(_this)->_healthCheck(); }
    void _healthCheck();
    TaskHandle_t _taskHealthCheckHandle = NULL;
    SemaphoreHandle_t _parameterMutex;
};

#endif // HEARTBEAT_WATCHDOG_H