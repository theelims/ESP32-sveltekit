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

#include <ESP32SvelteKit.h>

ESP32SvelteKit::ESP32SvelteKit(AsyncWebServer *server) : _featureService(server),
                                                         _securitySettingsService(server, &ESPFS),
                                                         _wifiSettingsService(server, &ESPFS, &_securitySettingsService, &_notificationEvents),
                                                         _wifiScanner(server, &_securitySettingsService),
                                                         _wifiStatus(server, &_securitySettingsService),
                                                         _apSettingsService(server, &ESPFS, &_securitySettingsService),
                                                         _apStatus(server, &_securitySettingsService, &_apSettingsService),
                                                         _notificationEvents(server),
#if FT_ENABLED(FT_NTP)
                                                         _ntpSettingsService(server, &ESPFS, &_securitySettingsService),
                                                         _ntpStatus(server, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_OTA)
                                                         _otaSettingsService(server, &ESPFS, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_UPLOAD_FIRMWARE)
                                                         _uploadFirmwareService(server, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_DOWNLOAD_FIRMWARE)
                                                         _downloadFirmwareService(server, &_securitySettingsService, &_notificationEvents),
#endif
#if FT_ENABLED(FT_MQTT)
                                                         _mqttSettingsService(server, &ESPFS, &_securitySettingsService),
                                                         _mqttStatus(server, &_mqttSettingsService, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_SECURITY)
                                                         _authenticationService(server, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_SLEEP)
                                                         _sleepService(server, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_BATTERY)
                                                         _batteryService(&_notificationEvents),
#endif
                                                         _restartService(server, &_securitySettingsService),
                                                         _factoryResetService(server, &ESPFS, &_securitySettingsService),
                                                         _systemStatus(server, &_securitySettingsService)
{
#ifdef PROGMEM_WWW
    // Serve static resources from PROGMEM
    WWWData::registerRoutes(
        [server, this](const String &uri, const String &contentType, const uint8_t *content, size_t len)
        {
            ArRequestHandlerFunction requestHandler = [contentType, content, len](AsyncWebServerRequest *request)
            {
                AsyncWebServerResponse *response = request->beginResponse_P(200, contentType, content, len);
                response->addHeader("Content-Encoding", "gzip");
                request->send(response);
            };
            server->on(uri.c_str(), HTTP_GET, requestHandler);
            // Serving non matching get requests with "/index.html"
            // OPTIONS get a straight up 200 response
            if (uri.equals("/index.html"))
            {
                server->onNotFound([requestHandler](AsyncWebServerRequest *request)
                                   {
                    if (request->method() == HTTP_GET) {
                        requestHandler(request);
                    } else if (request->method() == HTTP_OPTIONS) {
                        // CORS Pre-flight
                        request->send(200);
                    } else {
                        request->send(404);
                    } });
            }
        });
#else
    // Serve static resources from /www/
    server->serveStatic("/_app/", ESPFS, "/www/_app/");
    server->serveStatic("/favicon.png", ESPFS, "/www/favicon.png");
    //  Serving all other get requests with "/www/index.htm"
    //  OPTIONS get a straight up 200 response
    server->onNotFound([](AsyncWebServerRequest *request)
                       {
        if (request->method() == HTTP_GET) {
            request->send(ESPFS, "/www/index.html");
        } else if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404);
        } });
#endif

// Enable CORS if required
#if defined(ENABLE_CORS)
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", CORS_ORIGIN);
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
#endif
}

void ESP32SvelteKit::begin()
{
    ESPFS.begin(true);

    _wifiSettingsService.begin();

    MDNS.begin(_wifiSettingsService.getHostname().c_str());
    MDNS.setInstanceName(_appName);
    MDNS.addService("http", "tcp", 80);
    MDNS.addServiceTxt("http", "tcp", "Firmware Version", FIRMWARE_VERSION);

    Serial.printf("Running Firmware Version: %s\n", FIRMWARE_VERSION);

    _apSettingsService.begin();
#if FT_ENABLED(FT_NTP)
    _ntpSettingsService.begin();
#endif
#if FT_ENABLED(FT_MQTT)
    _mqttSettingsService.begin();
#endif
#if FT_ENABLED(FT_SECURITY)
    _securitySettingsService.begin();
#endif
#if FT_ENABLED(FT_OTA)
    _otaSettingsService.begin();
#endif

    xTaskCreateUniversal(
        this->_loopImpl,        // Function that should be called
        "ESP32 SvelteKit Loop", // Name of the task (for debugging)
        4096,                   // Stack size (bytes)
        this,                   // Pass reference to this class instance
        (tskIDLE_PRIORITY + 1), // task priority
        NULL,                   // Task handle
        0                       // Pin to application core
    );
}

void ESP32SvelteKit::_loop()
{
    while (1)
    {
        _wifiSettingsService.loop(); // 30 seconds
        _apSettingsService.loop();   // 10 seconds
#if FT_ENABLED(FT_MQTT)
        _mqttSettingsService.loop(); // 5 seconds
#endif
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}
