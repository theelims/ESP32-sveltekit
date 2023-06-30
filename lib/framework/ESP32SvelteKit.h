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
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Arduino.h>

#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <FeaturesService.h>
#include <APSettingsService.h>
#include <APStatus.h>
#include <AuthenticationService.h>
#include <BatteryService.h>
#include <FactoryResetService.h>
#include <MqttSettingsService.h>
#include <MqttStatus.h>
#include <NotificationEvents.h>
#include <NTPSettingsService.h>
#include <NTPStatus.h>
#include <OTASettingsService.h>
#include <UploadFirmwareService.h>
#include <RestartService.h>
#include <SecuritySettingsService.h>
#include <SleepService.h>
#include <SystemStatus.h>
#include <WiFiScanner.h>
#include <WiFiSettingsService.h>
#include <WiFiStatus.h>
#include <ESPFS.h>

#ifdef PROGMEM_WWW
#include <WWWData.h>
#endif

#ifndef CORS_ORIGIN
#define CORS_ORIGIN "*"
#endif

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "demo"
#endif

class ESP32SvelteKit
{
public:
    ESP32SvelteKit(AsyncWebServer *server);

    void begin();
    void loop();

    FS *getFS()
    {
        return &ESPFS;
    }

    SecurityManager *getSecurityManager()
    {
        return &_securitySettingsService;
    }

    NotificationEvents *getNotificationEvents()
    {
        return &_notificationEvents;
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

#if FT_ENABLED(FT_NTP)
    StatefulService<NTPSettings> *getNTPSettingsService()
    {
        return &_ntpSettingsService;
    }
#endif

#if FT_ENABLED(FT_OTA)
    StatefulService<OTASettings> *getOTASettingsService()
    {
        return &_otaSettingsService;
    }
#endif

#if FT_ENABLED(FT_MQTT)
    StatefulService<MqttSettings> *getMqttSettingsService()
    {
        return &_mqttSettingsService;
    }

    AsyncMqttClient *getMqttClient()
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
    FeaturesService _featureService;
    SecuritySettingsService _securitySettingsService;
    WiFiSettingsService _wifiSettingsService;
    WiFiScanner _wifiScanner;
    WiFiStatus _wifiStatus;
    APSettingsService _apSettingsService;
    APStatus _apStatus;
    NotificationEvents _notificationEvents;
#if FT_ENABLED(FT_NTP)
    NTPSettingsService _ntpSettingsService;
    NTPStatus _ntpStatus;
#endif
#if FT_ENABLED(FT_OTA)
    OTASettingsService _otaSettingsService;
#endif
#if FT_ENABLED(FT_UPLOAD_FIRMWARE)
    UploadFirmwareService _uploadFirmwareService;
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
    RestartService _restartService;
    FactoryResetService _factoryResetService;
    SystemStatus _systemStatus;
    String _appName = "ESP32 SvelteKit Demo";
};

#endif
