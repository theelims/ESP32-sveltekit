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

#include <HomeAssistant/HAService.h>
#include <HomeAssistant/HADevice.h>

HAService::HAService(PsychicMqttClient *mqttClient)
    : _mqttClient(mqttClient),
      _deviceName(APP_NAME),
      _manufacturer(""),
      _model(""),
      _discoveryPrefix(HA_DEFAULT_DISCOVERY_PREFIX),
      _enabled(false),
      _publishTaskHandle(nullptr)
{
    _generateDeviceId();
    _topicNamespace = _slugify(_deviceName);
}

// Defined here (not =default in the header) so that the unique_ptr<HADevice>
// member can see the complete HADevice type at destruction time.
HAService::~HAService() = default;

void HAService::begin()
{
    if (_publishTaskHandle != nullptr)
    {
        ESP_LOGW(TAG, "begin() called twice - ignoring");
        return;
    }

    BaseType_t ok = xTaskCreate(
        _publishTaskImpl,
        "ha_publish",
        6144,
        this,
        1,
        &_publishTaskHandle);

    if (ok != pdPASS || _publishTaskHandle == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create HA publish task");
        return;
    }

    if (_mqttClient != nullptr)
    {
        _mqttClient->onConnect([this](bool /*sessionPresent*/)
                               {
                                   if (_publishTaskHandle != nullptr)
                                   {
                                       xTaskNotifyGive(_publishTaskHandle);
                                   } });
    }

    ESP_LOGI(TAG, "HA publish task started, listening for MQTT connect notifications");
}

void HAService::_publishTaskImpl(void *param)
{
    static_cast<HAService *>(param)->_publishTask();
}

void HAService::_publishTask()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ESP_LOGI(TAG, "MQTT connected - invoking publishAll()");
        publishAll();
    }
}

// ============================================================================
// Configuration
// ============================================================================

void HAService::setDeviceName(const String &name)
{
    _deviceName = name;
    _topicNamespace = _slugify(name);
    _generateDeviceId();
    _syncMainDeviceIdentity();
}

void HAService::setManufacturer(const String &manufacturer)
{
    _manufacturer = manufacturer;
    _syncMainDeviceIdentity();
}

void HAService::setModel(const String &model)
{
    _model = model;
    _syncMainDeviceIdentity();
}

void HAService::setDiscoveryPrefix(const String &prefix)
{
    _discoveryPrefix = prefix;
    // Ensure prefix ends with /
    if (!_discoveryPrefix.endsWith("/"))
    {
        _discoveryPrefix += "/";
    }
}

void HAService::setEnabled(bool enabled)
{
    _enabled = enabled;
    ESP_LOGI(TAG, "HA integration %s", enabled ? "enabled" : "disabled");
}

// ============================================================================
// State queries
// ============================================================================

bool HAService::isReady() const
{
    return _enabled && _mqttClient != nullptr && _mqttClient->connected();
}

String HAService::getBaseTopic() const
{
    return _discoveryPrefix + _topicNamespace + "/" + _deviceId;
}

// ============================================================================
// MQTT Discovery helpers
// ============================================================================

void HAService::addDeviceInfo(JsonDocument &doc) const
{
    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = _deviceId;
    device["name"] = _deviceName;

    if (!_manufacturer.isEmpty())
    {
        device["manufacturer"] = _manufacturer;
    }

    if (!_model.isEmpty())
    {
        device["model"] = _model;
    }

    String swVersion = String(APP_VERSION);
    if (!swVersion.isEmpty())
    {
        device["sw_version"] = swVersion;
    }

    String hwVersion = String(HW_VERSION);
    if (!hwVersion.isEmpty())
    {
        device["hw_version"] = hwVersion;
    }

    // Add configuration URL if we have a valid IP
    IPAddress localIP = WiFi.localIP();
    if (IPUtils::isSet(localIP))
    {
        device["configuration_url"] = "http://" + localIP.toString() + "/";
    }
}

bool HAService::publishConfig(const String &component, const String &objectId, JsonDocument &config)
{
    if (!isReady())
        return false;

    // Set "~" abbreviation base
    config["~"] = getBaseTopic();

    // Add device info if not already present
    if (!config["device"].is<JsonObject>())
    {
        addDeviceInfo(config);
    }

    // Build discovery topic: {prefix}{component}/{deviceId}/{objectId}/config
    String configTopic = _discoveryPrefix + component + "/" + _deviceId + "/" + objectId + "/config";

    String payload;
    serializeJson(config, payload);

    int result = _mqttClient->publish(configTopic.c_str(), 0, true, payload.c_str(), 0, false);

    if (result != -1)
    {
        ESP_LOGI(TAG, "Published %s/%s config (%d bytes)", component.c_str(), objectId.c_str(), payload.length());
        return true;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to publish %s/%s config", component.c_str(), objectId.c_str());
        return false;
    }
}

bool HAService::publish(const String &topic, const String &payload, int qos, bool retain, bool async)
{
    if (_mqttClient == nullptr || !_mqttClient->connected())
        return false;

    return _mqttClient->publish(topic.c_str(), qos, retain, payload.c_str(), payload.length(), async) != -1;
}

void HAService::subscribe(const String &topic,
                          std::function<void(char *, char *, int, int, bool)> callback,
                          int qos)
{
    if (_mqttClient == nullptr)
        return;

    _mqttClient->onTopic(topic.c_str(), qos, callback);
}

// ============================================================================
// Entity lifecycle management
// ============================================================================

void HAService::onPublishAll(PublishAllCallback callback)
{
    _publishCallbacks.push_back(callback);
}

void HAService::onUnpublishAll(PublishAllCallback callback)
{
    _unpublishCallbacks.push_back(callback);
}

void HAService::publishAll()
{
    if (!_enabled)
        return;

    if (_mqttClient == nullptr || !_mqttClient->connected())
    {
        ESP_LOGW(TAG, "publishAll() called but MQTT is not connected");
        return;
    }

    ESP_LOGI(TAG, "Publishing all HA entities (%d callbacks, main+%d sub-devices)",
             _publishCallbacks.size(), (int)_subDevices.size());

    for (auto &callback : _publishCallbacks)
    {
        if (callback) callback();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    if (_mainDevice)
    {
        _syncMainDeviceIdentity();
        _mainDevice->publishAll();
    }

    for (auto &dev : _subDevices)
    {
        if (dev)
            dev->publishAll();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void HAService::unpublishAll()
{
    if (!_enabled)
        return;

    if (_mqttClient == nullptr || !_mqttClient->connected())
    {
        ESP_LOGW(TAG, "unpublishAll() called but MQTT is not connected");
        return;
    }

    ESP_LOGI(TAG, "Unpublishing all HA entities (%d callbacks, main+%d sub-devices)",
             _unpublishCallbacks.size(), (int)_subDevices.size());

    for (auto &callback : _unpublishCallbacks)
    {
        if (callback) callback();
    }

    if (_mainDevice)
        _mainDevice->unpublishAll();

    for (auto &dev : _subDevices)
    {
        if (dev)
            dev->unpublishAll();
    }
}

// ============================================================================
// Devices
// ============================================================================

HADevice &HAService::mainDevice()
{
    _ensureMainDevice();
    return *_mainDevice;
}

HADevice *HAService::addSubDevice(std::unique_ptr<HADevice> device)
{
    if (!device)
        return nullptr;

    // Wire in the main device relationship if not already set by the caller.
    HADeviceIdentity &id = device->identity();
    if (id.viaDevice.isEmpty())
        id.viaDevice = _deviceId;
    if (id.topicNamespace.isEmpty())
        id.topicNamespace = _topicNamespace + "/" + _deviceId;

    HADevice *raw = device.get();
    _subDevices.push_back(std::move(device));

    // Publish immediately when MQTT is already connected.
    if (isReady())
    {
        ESP_LOGI(TAG, "Adding sub-device '%s' - publishing immediately", raw->getDeviceId().c_str());
        raw->publishAll();
    }
    else
    {
        ESP_LOGI(TAG, "Adding sub-device '%s' - will publish on next connect", raw->getDeviceId().c_str());
    }

    return raw;
}

bool HAService::removeSubDevice(const String &deviceId)
{
    for (auto it = _subDevices.begin(); it != _subDevices.end(); ++it)
    {
        if (*it && (*it)->getDeviceId() == deviceId)
        {
            ESP_LOGI(TAG, "Removing sub-device '%s'", deviceId.c_str());
            (*it)->unpublishAll();
            _subDevices.erase(it);
            return true;
        }
    }

    ESP_LOGW(TAG, "removeSubDevice('%s'): device not found", deviceId.c_str());
    return false;
}

void HAService::_ensureMainDevice()
{
    if (_mainDevice)
        return;

    HADeviceIdentity identity;
    identity.id = _deviceId;
    identity.name = _deviceName;
    identity.manufacturer = _manufacturer;
    identity.model = _model;
    identity.swVersion = String(APP_VERSION);
    identity.hwVersion = String(HW_VERSION);
    identity.topicNamespace = _topicNamespace;

    _mainDevice = std::unique_ptr<HADevice>(new HADevice(this, std::move(identity)));
}

void HAService::_syncMainDeviceIdentity()
{
    if (!_mainDevice)
        return;

    HADeviceIdentity &id = _mainDevice->identity();
    id.id = _deviceId;
    id.name = _deviceName;
    id.manufacturer = _manufacturer;
    id.model = _model;
    id.swVersion = String(APP_VERSION);
    id.hwVersion = String(HW_VERSION);
    id.topicNamespace = _topicNamespace;

    IPAddress localIP = WiFi.localIP();
    if (IPUtils::isSet(localIP))
    {
        id.configurationUrl = "http://" + localIP.toString() + "/";
    }
}

// ============================================================================
// Private methods
// ============================================================================

void HAService::_generateDeviceId()
{
    _deviceId = _slugify(_deviceName) + "-" + SettingValue::getUniqueId();
}

String HAService::_slugify(const String &name)
{
    String slug = name;
    slug.toLowerCase();

    // Replace spaces and underscores with dashes
    slug.replace(" ", "-");
    slug.replace("_", "-");

    // Remove any characters that aren't alphanumeric or dash
    String clean;
    clean.reserve(slug.length());
    for (unsigned int i = 0; i < slug.length(); i++)
    {
        char c = slug.charAt(i);
        if (isalnum(c) || c == '-')
        {
            clean += c;
        }
    }

    // Remove consecutive dashes
    while (clean.indexOf("--") >= 0)
    {
        clean.replace("--", "-");
    }

    // Remove leading/trailing dashes
    while (clean.startsWith("-"))
        clean = clean.substring(1);
    while (clean.endsWith("-"))
        clean = clean.substring(0, clean.length() - 1);

    return clean;
}
