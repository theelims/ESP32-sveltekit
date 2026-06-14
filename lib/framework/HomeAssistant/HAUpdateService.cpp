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

#include <HomeAssistant/HAUpdateService.h>
#include <HomeAssistant/HAOTACallback.h>

// Static member initialization
std::atomic<bool> HAUpdateService::_otaUpdateInProgress{false};

HAUpdateService::HAUpdateService(HAService *haService,
                                 DownloadFirmwareService *downloadFirmwareService,
                                 EventSocket *eventSocket)
    : _haService(haService),
      _downloadFirmwareService(downloadFirmwareService),
      _eventSocket(eventSocket),
      _updateAvailable(false),
      _updateCheckTimer(nullptr),
      _updateCheckTaskHandle(nullptr),
      _initialUpdateCheckDone(false),
      _updateCheckCounter(0),
      _updateCheckPending(false)
{
}

void HAUpdateService::begin()
{
    // Register with HAService for entity publishing and removal
    _haService->onPublishAll([this]() { this->publishAll(); });
    _haService->onUnpublishAll([this]() { this->unpublishAll(); });

    // Create persistent task for update checks (single 8KB stack, reused)
    xTaskCreate(
        _updateCheckTask,
        "ha_update_chk",
        8192, // 8KB stack for HTTPS/TLS
        this,
        1, // Low priority
        &_updateCheckTaskHandle);

    if (_updateCheckTaskHandle != nullptr)
    {
        ESP_LOGI(TAG, "Update check task created");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create update check task");
    }

    // Register WiFi event handler for initial update check
    WiFi.onEvent(
        std::bind(&HAUpdateService::_onWiFiGotIP, this, std::placeholders::_1, std::placeholders::_2),
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

    // Create timer for periodic update checks
    _updateCheckTimer = xTimerCreate(
        "HAUpdateChk",
        pdMS_TO_TICKS(UPDATE_CHECK_TIMER_INTERVAL_MS),
        pdTRUE, // Auto-reload
        this,
        _updateCheckTimerCallback);

    if (_updateCheckTimer != nullptr)
    {
        xTimerStart(_updateCheckTimer, 0);
        uint32_t intervalHours = (UPDATE_CHECK_TIMER_INTERVAL_MS * UPDATE_CHECK_COUNTER_TARGET) / (60 * 60 * 1000);
        ESP_LOGI(TAG, "Update check timer started (%d min intervals, check every %d callbacks = %lu hours)",
                 UPDATE_CHECK_TIMER_INTERVAL_MS / (60 * 1000), UPDATE_CHECK_COUNTER_TARGET, intervalHours);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create update check timer");
    }
}

// ============================================================================
// Entity publishing
// ============================================================================

void HAUpdateService::unpublishAll()
{
    if (_haService == nullptr || !_haService->isReady())
        return;

    String configTopic = _haService->getDiscoveryPrefix() + "update/" +
                         _haService->getDeviceId() + "/firmware/config";
    _haService->publish(configTopic, String(), 0, true, false);

    String stateTopic = _haService->getBaseTopic() + "/update/state";
    _haService->publish(stateTopic, String(), 0, true, false);

    ESP_LOGI(TAG, "Unpublished update entity");
}

void HAUpdateService::publishAll()
{
    if (!_haService->isReady())
        return;

    _publishUpdateEntity();
    _publishUpdateState();
    _subscribeToCommands();
}

void HAUpdateService::_publishUpdateEntity()
{
    String baseTopic = _haService->getBaseTopic();

    JsonDocument config;
    config["name"] = "Firmware Update";
    config["unique_id"] = _haService->getDeviceId() + "_firmware_update";
    config["state_topic"] = "~/update/state";
    config["command_topic"] = "~/update/install";
    config["payload_install"] = "INSTALL";
    config["title"] = String(APP_NAME) + " Firmware";
    config["device_class"] = "firmware";
    config["icon"] = "mdi:cloud-download";

    if (_haService->publishConfig("update", "firmware", config))
    {
        ESP_LOGI(TAG, "Published update entity config");
    }
}

void HAUpdateService::_publishUpdateState()
{
    if (!_haService->isReady())
        return;

    String stateTopic = _haService->getBaseTopic() + "/update/state";

    String latestVer = !_latestVersion.isEmpty() ? _latestVersion : String(APP_VERSION);
    String releaseName = _latestReleaseName;

    JsonDocument state;
    _buildUpdateState(state, latestVer, releaseName);

    // Always include in_progress field per HA documentation
    state["in_progress"] = false;

    if (_updateAvailable)
    {
        ESP_LOGI(TAG, "Update available: %s -> %s", APP_VERSION, _latestVersion.c_str());
    }
    else
    {
        ESP_LOGV(TAG, "No update available (installed: %s, latest: %s)", APP_VERSION, latestVer.c_str());
    }

    String payload;
    serializeJson(state, payload);
    _haService->publish(stateTopic, payload);
}

void HAUpdateService::publishOTAProgress(int progress)
{
    if (!_haService->isReady())
        return;

    String stateTopic = _haService->getBaseTopic() + "/update/state";
    String latestVer = !_latestVersion.isEmpty() ? _latestVersion : String(APP_VERSION);
    String releaseName = _latestReleaseName;

    JsonDocument state;
    _buildUpdateState(state, latestVer, releaseName);

    if (progress >= 0 && progress <= 100)
    {
        state["in_progress"] = true;
        state["update_percentage"] = (float)progress;
    }

    String payload;
    serializeJson(state, payload);
    
    bool async = (progress != 100); // At 100%, publish synchronously to ensure message is sent before restart
    _haService->publish(stateTopic, payload, 0, false, async);
}

void HAUpdateService::publishOTAError()
{
    if (!_haService->isReady())
        return;

    // Reset to idle state after failed OTA
    _publishUpdateState();
}

void HAUpdateService::_buildUpdateState(JsonDocument &state, const String &latestVersion, const String &releaseName)
{
    state["installed_version"] = String(APP_VERSION);
    state["latest_version"] = latestVersion;
    
    // Use GitHub release name if available, otherwise fall back to generic title
    if (!releaseName.isEmpty()) {
        state["title"] = releaseName;
    } else {
        state["title"] = String(APP_NAME) + " Firmware";
    }
    
    state["release_url"] = "https://github.com/" + String(GITHUB_REPO_OWNER) + "/" + String(GITHUB_REPO_NAME) + "/releases/latest";
}

// ============================================================================
// Command handlers
// ============================================================================

void HAUpdateService::_subscribeToCommands()
{
    String baseTopic = _haService->getBaseTopic();
    String installTopic = baseTopic + "/update/install";

    _haService->subscribe(installTopic,
                          [this](char *topic, char *payload, int retain, int qos, bool dup)
                          {
                              this->_onUpdateInstallCommand(topic, payload, retain, qos, dup);
                          });

    ESP_LOGV(TAG, "Subscribed to update install command");
}

void HAUpdateService::_onUpdateInstallCommand(char *topic, char *payload, int retain, int qos, bool dup)
{
    if (!_updateAvailable || _downloadUrl.isEmpty())
    {
        ESP_LOGW(TAG, "Update install requested but no update is available");
        return;
    }

    // Try to acquire OTA flag using atomic compare-exchange
    bool expected = false;
    if (!_otaUpdateInProgress.compare_exchange_strong(expected, true))
    {
        ESP_LOGW(TAG, "OTA update already in progress");
        return;
    }

    ESP_LOGI(TAG, "Update install requested via MQTT - starting OTA to version %s", _latestVersion.c_str());

    // Create callback for dual-channel progress reporting (MQTT + WebSocket)
    HAOTACallback *callback = new HAOTACallback(
        this,
        _eventSocket,
        [this]()
        { _otaUpdateInProgress.store(false); } // Error cleanup
    );

    // Start OTA update (graceful reboot handled by DownloadFirmwareService)
    if (!DownloadFirmwareService::startOTAUpdate(_downloadUrl, callback))
    {
        ESP_LOGE(TAG, "Failed to start OTA update");

        // Report error to WebSocket
        if (_eventSocket != nullptr)
        {
            JsonDocument doc;
            doc["status"] = "error";
            doc["error"] = "Failed to create OTA task";
            JsonObject jsonObject = doc.as<JsonObject>();
            _eventSocket->emitEvent(EVENT_OTA_UPDATE, jsonObject);
        }

        _otaUpdateInProgress.store(false);
        delete callback;
        return;
    }
}

// ============================================================================
// Update checking
// ============================================================================

void HAUpdateService::checkForUpdates()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        ESP_LOGW(TAG, "Skipping update check - WiFi not connected");
        return;
    }

    ESP_LOGI(TAG, "Checking for firmware updates...");

    if (_queryGitHubForLatestRelease())
    {
        if (_haService->isReady())
        {
            _publishUpdateState();
        }
    }
}

bool HAUpdateService::_queryGitHubForLatestRelease()
{
    String userAgent = String(APP_NAME) + "/" + String(APP_VERSION);

    GitHubReleaseInfo releaseInfo = GitHubReleaseService::queryLatestRelease(
        String(GITHUB_REPO_OWNER),
        String(GITHUB_REPO_NAME),
        userAgent,
        false // Verify SSL certificates
    );

    if (!releaseInfo.valid)
    {
        ESP_LOGE(TAG, "Failed to query GitHub for latest release");
        return false;
    }

    _latestVersion = releaseInfo.version;
    _latestReleaseName = releaseInfo.name;
    _downloadUrl = releaseInfo.downloadUrl;

    String currentVersion = String(APP_VERSION);
    _updateAvailable = GitHubReleaseService::isNewerVersion(currentVersion, _latestVersion);

    ESP_LOGI(TAG, "GitHub check complete - Current: %s, Latest: %s, Update: %s",
             currentVersion.c_str(), _latestVersion.c_str(), _updateAvailable ? "YES" : "NO");

    return true;
}

void HAUpdateService::_onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (!_initialUpdateCheckDone && _updateCheckTaskHandle != nullptr)
    {
        _initialUpdateCheckDone = true;
        ESP_LOGI(TAG, "WiFi connected (initial boot), triggering first update check");
        xTaskNotifyGive(_updateCheckTaskHandle);
    }
}

void HAUpdateService::_updateCheckTimerCallback(TimerHandle_t timer)
{
    HAUpdateService *service = static_cast<HAUpdateService *>(pvTimerGetTimerID(timer));
    if (service != nullptr && service->_updateCheckTaskHandle != nullptr)
    {
        service->_updateCheckCounter++;

        if (service->_updateCheckCounter >= UPDATE_CHECK_COUNTER_TARGET)
        {
            service->_updateCheckCounter = 0;
            ESP_LOGI(TAG, "Triggering periodic firmware update check");
            xTaskNotifyGive(service->_updateCheckTaskHandle);
        }
    }
}

void HAUpdateService::_updateCheckTask(void *parameter)
{
    HAUpdateService *service = static_cast<HAUpdateService *>(parameter);

    if (service == nullptr)
    {
        vTaskDelete(nullptr);
        return;
    }

    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (!service->_updateCheckPending)
        {
            service->_updateCheckPending = true;
            service->checkForUpdates();
            service->_updateCheckPending = false;
        }
        else
        {
            ESP_LOGW(TAG, "Update check already pending - skipping");
        }
    }
}
