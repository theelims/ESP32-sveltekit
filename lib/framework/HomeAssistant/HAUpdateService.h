#pragma once

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2025 theelims
 *   Copyright (C) 2026 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Arduino.h>
#include <atomic>
#include <HomeAssistant/HAService.h>
#include <DownloadFirmwareService.h>
#include <GitHubReleaseService.h>
#include <EventSocket.h>
#include <FirmwareUpdateEvents.h>


/**
 * @brief Home Assistant firmware update entity service
 *
 * Provides:
 * - HA MQTT Discovery for "update" entity (firmware management)
 * - Periodic GitHub release checking (timer + persistent FreeRTOS task)
 * - Install command handling (OTA via DownloadFirmwareService)
 * - Progress reporting to both MQTT and WebSocket
 *
 * Registers itself with HAService via onPublishAll() for automatic
 * entity publishing on MQTT connect.
 */
class HAUpdateService
{
public:
    static constexpr const char *TAG = "HAUpdateService";

    /// Timer fires every 30 minutes
    static constexpr uint32_t UPDATE_CHECK_TIMER_INTERVAL_MS = 30 * 60 * 1000;
    /// Perform actual check every 12 callbacks (= 6 hours)
    static constexpr uint8_t UPDATE_CHECK_COUNTER_TARGET = 12;

    /**
     * @brief Constructor
     * @param haService Shared HAService for device identity and MQTT helpers
     * @param downloadFirmwareService OTA download engine
     * @param eventSocket WebSocket for Web UI progress reporting
     */
    HAUpdateService(HAService *haService,
                    DownloadFirmwareService *downloadFirmwareService,
                    EventSocket *eventSocket);

    /**
     * @brief Initialize the service
     *
     * Creates persistent update check task, timer, and registers
     * with HAService for entity publishing on MQTT connect.
     * Registers WiFi event handler for initial boot update check.
     */
    void begin();

    /**
     * @brief Publish update entity config and current state
     *
     * Called by HAService::publishAll() on MQTT connect.
     */
    void publishAll();

    /**
     * @brief Remove the update entity from Home Assistant
     *
     * Sends empty retained payloads to the discovery config topic and the
     * state topic. Called by HAService::unpublishAll() when HA is disabled.
     */
    void unpublishAll();

    /**
     * @brief Check for firmware updates on GitHub
     *
     * Queries GitHub API for latest release and compares with current version.
     * Updates internal state and publishes to MQTT if connected.
     */
    void checkForUpdates();

    /**
     * @brief Get the atomic OTA-in-progress flag
     *
     * Can be checked by other services to prevent concurrent OTA updates.
     * @return Reference to the atomic flag
     */
    static std::atomic<bool> &getOtaInProgressFlag() { return _otaUpdateInProgress; }

    // ========================================================================
    // OTA MQTT state publishing (called by HAOTACallback)
    // ========================================================================

    /**
     * @brief Publish OTA progress to MQTT state topic
     * @param progress Percentage (0-100)
     */
    void publishOTAProgress(int progress);

    /**
     * @brief Publish OTA error (idle) state to MQTT state topic
     *
     * Resets the update entity back to idle after a failed OTA.
     */
    void publishOTAError();

private:
    HAService *_haService;
    DownloadFirmwareService *_downloadFirmwareService;
    EventSocket *_eventSocket;

    // Update state
    String _latestVersion;
    String _latestReleaseName;
    String _downloadUrl;
    bool _updateAvailable;

    // Timer and task
    TimerHandle_t _updateCheckTimer;
    TaskHandle_t _updateCheckTaskHandle;
    bool _initialUpdateCheckDone;
    uint8_t _updateCheckCounter;
    bool _updateCheckPending;

    // OTA coordination
    static std::atomic<bool> _otaUpdateInProgress;

    // ========================================================================
    // Entity publishing
    // ========================================================================

    /** @brief Publish update entity discovery config */
    void _publishUpdateEntity();

    /** @brief Publish current update state (installed version, available version) */
    void _publishUpdateState();

    /** @brief Populate common HA update state fields into a JsonDocument */
    void _buildUpdateState(JsonDocument &state, const String &latestVersion, const String &releaseName);

    // ========================================================================
    // Command handlers
    // ========================================================================

    /** @brief Handle update install command from HA */
    void _onUpdateInstallCommand(char *topic, char *payload, int retain, int qos, bool dup);

    /** @brief Subscribe to command topics */
    void _subscribeToCommands();

    // ========================================================================
    // Update checking
    // ========================================================================

    /** @brief Query GitHub API for latest release */
    bool _queryGitHubForLatestRelease();

    /** @brief WiFi event handler for initial update check after boot */
    void _onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);

    /** @brief Static timer callback */
    static void _updateCheckTimerCallback(TimerHandle_t timer);

    /** @brief Static persistent task for update checks */
    static void _updateCheckTask(void *parameter);
};
