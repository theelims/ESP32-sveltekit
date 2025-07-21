/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <MqttSettingsService.h>

/**
 * Load the root certificate bundle embedded by the build process
 */
extern const uint8_t rootca_crt_bundle_start[] asm("_binary_src_certs_x509_crt_bundle_bin_start");
extern const uint8_t rootca_crt_bundle_end[] asm("_binary_src_certs_x509_crt_bundle_bin_end");

/**
 * Retains a copy of the cstr provided in the pointer provided using dynamic allocation.
 *
 * Frees the pointer before allocation and leaves it as nullptr if cstr == nullptr.
 */
static char *retainCstr(const char *cstr, char **ptr)
{
    // free up previously retained value if exists
    free(*ptr);
    *ptr = nullptr;

    // dynamically allocate and copy cstr (if non null)
    if (cstr != nullptr)
    {
        *ptr = (char *)malloc(strlen(cstr) + 1);
        strcpy(*ptr, cstr);
    }

    // return reference to pointer for convenience
    return *ptr;
}

MqttSettingsService::MqttSettingsService(PsychicHttpServer *server,
                                         FS *fs,
                                         SecurityManager *securityManager) : _server(server),
                                                                             _securityManager(securityManager),
                                                                             _httpEndpoint(MqttSettings::read, MqttSettings::update, this, server, MQTT_SETTINGS_SERVICE_PATH, securityManager),
                                                                             _fsPersistence(MqttSettings::read, MqttSettings::update, this, fs, MQTT_SETTINGS_FILE),
                                                                             _retainedHost(nullptr),
                                                                             _retainedClientId(nullptr),
                                                                             _retainedUsername(nullptr),
                                                                             _retainedPassword(nullptr),
                                                                             _reconfigureMqtt(false),
                                                                             _mqttClient(),
                                                                             _lastError("None")
{
    String status_topic = SettingValue::format(FACTORY_MQTT_STATUS_TOPIC);
    retainCstr(status_topic.c_str(), &_retainedWillTopic);
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);

#if ESP_ARDUINO_VERSION_MAJOR == 3
    _mqttClient.setCACertBundle(rootca_crt_bundle_start, rootca_crt_bundle_end - rootca_crt_bundle_start);
#else
    _mqttClient.setCACertBundle(rootca_crt_bundle_start);
#endif
}

MqttSettingsService::~MqttSettingsService()
{
}

void MqttSettingsService::begin()
{
    WiFi.onEvent(
        std::bind(&MqttSettingsService::onStationModeDisconnected, this, std::placeholders::_1, std::placeholders::_2),
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(std::bind(&MqttSettingsService::onStationModeGotIP, this, std::placeholders::_1, std::placeholders::_2),
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    _mqttClient.onConnect(std::bind(&MqttSettingsService::onMqttConnect, this, std::placeholders::_1));
    _mqttClient.onDisconnect(std::bind(&MqttSettingsService::onMqttDisconnect, this, std::placeholders::_1));
    _mqttClient.onError(std::bind(&MqttSettingsService::onMqttError, this, std::placeholders::_1));

    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}

void MqttSettingsService::loop()
{
    if (_reconfigureMqtt)
    {
        // reconfigure MQTT client
        configureMqtt();

        // clear the reconnection flags
        _reconfigureMqtt = false;
    }
}

bool MqttSettingsService::isEnabled()
{
    return _state.enabled;
}

bool MqttSettingsService::isConnected()
{
    return _mqttClient.connected();
}

const char *MqttSettingsService::getClientId()
{
    // return _mqttClient.getClientId();
    return _state.clientId.c_str();
}

PsychicMqttClient *MqttSettingsService::getMqttClient()
{
    return &_mqttClient;
}

String MqttSettingsService::getLastError()
{
    return _lastError;
}

void MqttSettingsService::onMqttConnect(bool sessionPresent)
{

#if ESP_IDF_VERSION_MAJOR == 5
    String uri = _mqttClient.getMqttConfig()->broker.address.uri;
#else
    String uri = _mqttClient.getMqttConfig()->uri;
#endif

    ESP_LOGI(SVK_TAG, "Connected to MQTT: %s", uri.c_str());
#ifdef SERIAL_INFO
    Serial.printf("Connected to MQTT: %s\n", uri.c_str());
#endif
    _lastError = "None";
    // publish status message
    _mqttClient.publish(_retainedWillTopic, 1, true, "online");
}

void MqttSettingsService::onMqttDisconnect(bool sessionPresent)
{
    ESP_LOGI(SVK_TAG, "Disconnected from MQTT.");
#ifdef SERIAL_INFO
    Serial.println("Disconnected from MQTT.");
#endif
}

void MqttSettingsService::onMqttError(esp_mqtt_error_codes_t error)
{
    if (error.error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
    {
        _lastError = strerror(error.esp_transport_sock_errno);
        ESP_LOGE(SVK_TAG, "MQTT TCP error: %s", _lastError.c_str());
    }
}

void MqttSettingsService::onConfigUpdated()
{
    _reconfigureMqtt = true;
}

void MqttSettingsService::onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (_state.enabled)
    {
        ESP_LOGI(SVK_TAG, "WiFi connection established, starting MQTT client.");
        onConfigUpdated();
    }
}

void MqttSettingsService::onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (_state.enabled)
    {
        ESP_LOGI(SVK_TAG, "WiFi connection dropped, stopping MQTT client.");
        onConfigUpdated();
    }
}

void MqttSettingsService::configureMqtt()
{
    disconnect();

    // only connect if WiFi is connected and MQTT is enabled
    if (_state.enabled && WiFi.isConnected())
    {
#ifdef SERIAL_INFO
        Serial.println("Connecting to MQTT...");
#endif
        _mqttClient.setServer(retainCstr(_state.uri.c_str(), &_retainedHost));
        if (_state.username.length() > 0)
        {
            _mqttClient.setCredentials(
                retainCstr(_state.username.c_str(), &_retainedUsername),
                retainCstr(_state.password.length() > 0 ? _state.password.c_str() : nullptr, &_retainedPassword));
        }
        else
        {
            _mqttClient.setCredentials(retainCstr(nullptr, &_retainedUsername), retainCstr(nullptr, &_retainedPassword));
        }
        _mqttClient.setClientId(retainCstr(_state.clientId.c_str(), &_retainedClientId));
        _mqttClient.setKeepAlive(_state.keepAlive);
        _mqttClient.setWill(_retainedWillTopic, 1, true, _retainedWillPayload);
        _mqttClient.setCleanSession(_state.cleanSession);
        _mqttClient.connect();

        MqttCommitHandler::setTimerInterval(_state.messageIntervalMs);
    }
}

void MqttSettingsService::setStatusTopic(String statusTopic)
{
    // check if the status topic is different from the current one
    if (statusTopic.equals(_retainedWillTopic))
    {
        return; // no change
    }

    // copy the new status topic to the retained pointer
    retainCstr(statusTopic.c_str(), &_retainedWillTopic);

    // update the state
    _reconfigureMqtt = true; // mark for reconfiguration

    ESP_LOGI(SVK_TAG, "Status topic updated to: %s", _retainedWillTopic);
}

String MqttSettingsService::getStatusTopic()
{
    return String(_retainedWillTopic);
}

void MqttSettingsService::disconnect()
{
    // disable MQTT message commit timer, if disconnected
    MqttCommitHandler::setTimerInterval(0);

    // disconnect if currently connected
    if (_mqttClient.connected())
    {
        ESP_LOGI(SVK_TAG, "Disconnecting from MQTT client.");
        _mqttClient.publish(_retainedWillTopic, 1, true, "offline", 0, false);
        _mqttClient.disconnect();
    }
}
