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

#include <NTPSettingsService.h>

NTPSettingsService::NTPSettingsService(PsychicHttpServer *server,
                                       FS *fs,
                                       SecurityManager *securityManager) : _server(server),
                                                                           _securityManager(securityManager),
                                                                           _httpEndpoint(NTPSettings::read, NTPSettings::update, this, server, NTP_SETTINGS_SERVICE_PATH, securityManager),
                                                                           _fsPersistence(NTPSettings::read, NTPSettings::update, this, fs, NTP_SETTINGS_FILE)
{
    addUpdateHandler([&](const String &originId)
                     { configureNTP(); },
                     false);
}

void NTPSettingsService::begin()
{
    WiFi.onEvent(
        std::bind(&NTPSettingsService::onStationModeDisconnected, this, std::placeholders::_1, std::placeholders::_2),
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(std::bind(&NTPSettingsService::onStationModeGotIP, this, std::placeholders::_1, std::placeholders::_2),
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

    _httpEndpoint.begin();
    _server->on(TIME_PATH,
                HTTP_POST,
                _securityManager->wrapCallback(
                    std::bind(&NTPSettingsService::configureTime, this, std::placeholders::_1, std::placeholders::_2),
                    AuthenticationPredicates::IS_ADMIN));

    ESP_LOGV("NTPSettingsService", "Registered POST endpoint: %s", TIME_PATH);

    _fsPersistence.readFromFS();
    configureNTP();
}

void NTPSettingsService::onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
#ifdef SERIAL_INFO
    Serial.println(F("Got IP address, starting NTP Synchronization"));
#endif
    configureNTP();
}

void NTPSettingsService::onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
#ifdef SERIAL_INFO
    Serial.println(F("WiFi connection dropped, stopping NTP."));
#endif
    configureNTP();
}

void NTPSettingsService::configureNTP()
{
    if (WiFi.isConnected() && _state.enabled)
    {
#ifdef SERIAL_INFO
        Serial.println(F("Starting NTP..."));
#endif
        configTzTime(_state.tzFormat.c_str(), _state.server.c_str());
    }
    else
    {
        setenv("TZ", _state.tzFormat.c_str(), 1);
        tzset();
        sntp_stop();
    }
}

esp_err_t NTPSettingsService::configureTime(PsychicRequest *request, JsonVariant &json)
{
    if (!sntp_enabled() && json.is<JsonObject>())
    {
        struct tm tm = {0};
        String timeLocal = json["local_time"];
        char *s = strptime(timeLocal.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
        if (s != nullptr)
        {
            time_t time = mktime(&tm);
            struct timeval now = {.tv_sec = time};
            settimeofday(&now, nullptr);
            return request->reply(200);
        }
    }
    return request->reply(400);
}
