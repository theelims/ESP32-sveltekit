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

#include <OTASettingsService.h>

OTASettingsService::OTASettingsService(AsyncWebServer *server, FS *fs, SecurityManager *securityManager) : _httpEndpoint(OTASettings::read, OTASettings::update, this, server, OTA_SETTINGS_SERVICE_PATH, securityManager),
                                                                                                           _fsPersistence(OTASettings::read, OTASettings::update, this, fs, OTA_SETTINGS_FILE),
                                                                                                           _arduinoOTA(nullptr)
{
    WiFi.onEvent(std::bind(&OTASettingsService::onStationModeGotIP, this, std::placeholders::_1, std::placeholders::_2),
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    addUpdateHandler([&](const String &originId)
                     { configureArduinoOTA(); },
                     false);
}

void OTASettingsService::begin()
{
    _fsPersistence.readFromFS();
    configureArduinoOTA();
}

void OTASettingsService::_loop()
{
    while (1)
    {
        _arduinoOTA->handle();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void OTASettingsService::configureArduinoOTA()
{
    if (_arduinoOTA)
    {
        _arduinoOTA->end();
        delete _arduinoOTA;
        _arduinoOTA = nullptr;
        MDNS.disableArduino();
        Serial.println(F("Ending OTA Update Service"));
        if (_loopHandle != NULL)
        {
            vTaskDelete(_loopHandle);
            _loopHandle = NULL;
        }
    }
    if (_state.enabled)
    {
        Serial.println(F("Starting OTA Update Service..."));
        _arduinoOTA = new ArduinoOTAClass;
        _arduinoOTA->setPort(_state.port);
        _arduinoOTA->setPassword(_state.password.c_str());
        _arduinoOTA->setMdnsEnabled(false);
        _arduinoOTA->onStart([]()
                             { Serial.println(F("Starting")); });
        _arduinoOTA->onEnd([]()
                           { Serial.println(F("\r\nEnd")); });
        _arduinoOTA->onProgress([](unsigned int progress, unsigned int total)
                                { Serial.printf_P(PSTR("Progress: %u%%\r\n"), (progress / (total / 100))); });
        _arduinoOTA->onError([](ota_error_t error)
                             {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR)
        Serial.println(F("Auth Failed"));
      else if (error == OTA_BEGIN_ERROR)
        Serial.println(F("Begin Failed"));
      else if (error == OTA_CONNECT_ERROR)
        Serial.println(F("Connect Failed"));
      else if (error == OTA_RECEIVE_ERROR)
        Serial.println(F("Receive Failed"));
      else if (error == OTA_END_ERROR)
        Serial.println(F("End Failed")); });

        _arduinoOTA->begin();
        MDNS.enableArduino(_state.port, (_state.password.length() > 0));

        if (_loopHandle == NULL)
        {
            xTaskCreateUniversal(
                this->_loopImpl,        // Function that should be called
                "ArduinoOTA loop",      // Name of the task (for debugging)
                4096,                   // Stack size (bytes)
                this,                   // Pass reference to this class instance
                (tskIDLE_PRIORITY + 2), // task priority
                &_loopHandle,           // Task handle
                0                       // Pin to application core
            );
        }
        else
        {
            vTaskResume(_loopHandle);
        }
    }
}

void OTASettingsService::onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    configureArduinoOTA();
}
