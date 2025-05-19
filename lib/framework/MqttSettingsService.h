#ifndef MqttSettingsService_h
#define MqttSettingsService_h

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

#include <StatefulService.h>
#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <PsychicMqttClient.h>
#include <SettingValue.h>
#include <WiFi.h>

#ifndef FACTORY_MQTT_ENABLED
#define FACTORY_MQTT_ENABLED false
#endif

#ifndef FACTORY_MQTT_HOST
#define FACTORY_MQTT_HOST "test.mosquitto.org"
#endif

#ifndef FACTORY_MQTT_PORT
#define FACTORY_MQTT_PORT 1883
#endif

#ifndef FACTORY_MQTT_USERNAME
#define FACTORY_MQTT_USERNAME ""
#endif

#ifndef FACTORY_MQTT_PASSWORD
#define FACTORY_MQTT_PASSWORD ""
#endif

#ifndef FACTORY_MQTT_CLIENT_ID
#define FACTORY_MQTT_CLIENT_ID "#{platform}-#{unique_id}"
#endif

#ifndef FACTORY_MQTT_KEEP_ALIVE
#define FACTORY_MQTT_KEEP_ALIVE 16
#endif

#ifndef FACTORY_MQTT_CLEAN_SESSION
#define FACTORY_MQTT_CLEAN_SESSION true
#endif

#ifndef FACTORY_MQTT_MAX_TOPIC_LENGTH
#define FACTORY_MQTT_MAX_TOPIC_LENGTH 128
#endif

#define MQTT_SETTINGS_FILE "/config/mqttSettings.json"
#define MQTT_SETTINGS_SERVICE_PATH "/rest/mqttSettings"

#define MQTT_RECONNECTION_DELAY 5000

class MqttSettings
{
public:
    // host and port - if enabled
    bool enabled;
    String uri;

    // username and password
    String username;
    String password;

    // client id settings
    String clientId;

    // connection settings
    uint16_t keepAlive;
    bool cleanSession;

    static void read(MqttSettings &settings, JsonObject &root)
    {
        root["enabled"] = settings.enabled;
        root["uri"] = settings.uri;
        root["username"] = settings.username;
        root["password"] = settings.password;
        root["client_id"] = settings.clientId;
        root["keep_alive"] = settings.keepAlive;
        root["clean_session"] = settings.cleanSession;
    }

    static StateUpdateResult update(JsonObject &root, MqttSettings &settings)
    {
        settings.enabled = root["enabled"] | FACTORY_MQTT_ENABLED;
        settings.uri = root["uri"] | FACTORY_MQTT_URI;
        settings.username = root["username"] | SettingValue::format(FACTORY_MQTT_USERNAME);
        settings.password = root["password"] | FACTORY_MQTT_PASSWORD;
        settings.clientId = root["client_id"] | SettingValue::format(FACTORY_MQTT_CLIENT_ID);
        settings.keepAlive = root["keep_alive"] | FACTORY_MQTT_KEEP_ALIVE;
        settings.cleanSession = root["clean_session"] | FACTORY_MQTT_CLEAN_SESSION;
        return StateUpdateResult::CHANGED;
    }
};

class MqttSettingsService : public StatefulService<MqttSettings>
{
public:
    MqttSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager);
    ~MqttSettingsService();

    void begin();
    void loop();
    bool isEnabled();
    bool isConnected();
    const char *getClientId();
    String getLastError();
    PsychicMqttClient *getMqttClient();

protected:
    void onConfigUpdated();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    HttpEndpoint<MqttSettings> _httpEndpoint;
    FSPersistence<MqttSettings> _fsPersistence;

    // Pointers to hold retained copies of the mqtt client connection strings.
    // This is required as AsyncMqttClient holds references to the supplied connection strings.
    char *_retainedHost;
    char *_retainedClientId;
    char *_retainedUsername;
    char *_retainedPassword;

    // variable to help manage connection
    bool _reconfigureMqtt;
    String _lastError;

    // the MQTT client instance
    PsychicMqttClient _mqttClient;

    void onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    void onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(bool sessionPresent);
    void onMqttError(esp_mqtt_error_codes_t error);
    void configureMqtt();
};

#endif // end MqttSettingsService_h
