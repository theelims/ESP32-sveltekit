#ifndef ESP32SvelteKit_h
#define ESP32SvelteKit_h

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

#include <WiFi.h>
#include <ESPmDNS.h>
#include <AnalyticsService.h>
#include <FeaturesService.h>
#include <APSettingsService.h>
#include <APStatus.h>
#include <AuthenticationService.h>
#include <BatteryService.h>
#include <FactoryResetService.h>
#include <DownloadFirmwareService.h>
#include <EventSocket.h>
#include <MqttSettingsService.h>
#include <MqttStatus.h>
#include <NotificationService.h>
#include <NTPSettingsService.h>
#include <NTPStatus.h>
#include <UploadFirmwareService.h>
#include <RestartService.h>
#include <SecuritySettingsService.h>
#include <SleepService.h>
#include <SystemStatus.h>
#include <WiFiScanner.h>
#include <WiFiSettingsService.h>
#include <WiFiStatus.h>
#include <ESPFS.h>
#include <PsychicHttp.h>

#ifdef EMBED_WWW
#include <WWWData.h>
#endif

#ifndef CORS_ORIGIN
#define CORS_ORIGIN "*"
#endif

#ifndef APP_VERSION
#define APP_VERSION "demo"
#endif

#ifndef APP_NAME
#define APP_NAME "ESP32 SvelteKit Demo"
#endif

#ifndef ESP32SVELTEKIT_RUNNING_CORE
#define ESP32SVELTEKIT_RUNNING_CORE -1
#endif

class ESP32SvelteKit
{
public:
    ESP32SvelteKit(PsychicHttpServer *server, unsigned int numberEndpoints = 115);

    void begin();

    FS *getFS()
    {
        return &ESPFS;
    }

    PsychicHttpServer *getServer()
    {
        return _server;
    }

    SecurityManager *getSecurityManager()
    {
        return &_securitySettingsService;
    }

    EventSocket *getSocket()
    {
        return &_socket;
    }

#if FT_ENABLED(FT_SECURITY)
    StatefulService<SecuritySettings> *getSecuritySettingsService()
    {
        return &_securitySettingsService;
    }
#endif

    StatefulService<WiFiSettings> *getWiFiSettingsService()
    {
        return &_wifiSettingsService;
    }

    StatefulService<APSettings> *getAPSettingsService()
    {
        return &_apSettingsService;
    }

    NotificationService *getNotificationService()
    {
        return &_notificationService;
    }

#if FT_ENABLED(FT_NTP)
    StatefulService<NTPSettings> *getNTPSettingsService()
    {
        return &_ntpSettingsService;
    }
#endif

#if FT_ENABLED(FT_MQTT)
    StatefulService<MqttSettings> *getMqttSettingsService()
    {
        return &_mqttSettingsService;
    }

    PsychicMqttClient *getMqttClient()
    {
        return _mqttSettingsService.getMqttClient();
    }
#endif

#if FT_ENABLED(FT_SLEEP)
    SleepService *getSleepService()
    {
        return &_sleepService;
    }
#endif

#if FT_ENABLED(FT_BATTERY)
    BatteryService *getBatteryService()
    {
        return &_batteryService;
    }
#endif

    FeaturesService *getFeatureService()
    {
        return &_featureService;
    }

    void factoryReset()
    {
        _factoryResetService.factoryReset();
    }

    void setMDNSAppName(String name)
    {
        _appName = name;
    }

    void recoveryMode()
    {
        _apSettingsService.recoveryMode();
    }

private:
    PsychicHttpServer *_server;
    unsigned int _numberEndpoints;
    FeaturesService _featureService;
    SecuritySettingsService _securitySettingsService;
    WiFiSettingsService _wifiSettingsService;
    WiFiScanner _wifiScanner;
    WiFiStatus _wifiStatus;
    APSettingsService _apSettingsService;
    APStatus _apStatus;
    EventSocket _socket;
    NotificationService _notificationService;
#if FT_ENABLED(FT_NTP)
    NTPSettingsService _ntpSettingsService;
    NTPStatus _ntpStatus;
#endif
#if FT_ENABLED(FT_UPLOAD_FIRMWARE)
    UploadFirmwareService _uploadFirmwareService;
#endif
#if FT_ENABLED(FT_DOWNLOAD_FIRMWARE)
    DownloadFirmwareService _downloadFirmwareService;
#endif
#if FT_ENABLED(FT_MQTT)
    MqttSettingsService _mqttSettingsService;
    MqttStatus _mqttStatus;
#endif
#if FT_ENABLED(FT_SECURITY)
    AuthenticationService _authenticationService;
#endif
#if FT_ENABLED(FT_SLEEP)
    SleepService _sleepService;
#endif
#if FT_ENABLED(FT_BATTERY)
    BatteryService _batteryService;
#endif
#if FT_ENABLED(FT_ANALYTICS)
    AnalyticsService _analyticsService;
#endif
    RestartService _restartService;
    FactoryResetService _factoryResetService;
    SystemStatus _systemStatus;

    String _appName = APP_NAME;

protected:
    static void _loopImpl(void *_this) { static_cast<ESP32SvelteKit *>(_this)->_loop(); }
    void _loop();
};

#endif
