#ifndef LightSettingsService_h
#define LightSettingsService_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *   Copyright (C) 2026 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <ESP32SvelteKit.h>
#include <EventEndpoint.h>
#include <FSPersistence.h>
#include <HttpEndpoint.h>
#include <HomeAssistant/HAGroupedSwitchPublisher.h>

#define LIGHT_SETTINGS_REST_ENDPOINT "/rest/lightSettings"
#define LIGHT_SETTINGS_SOCKET_EVENT  "lightSettings"
#define LIGHT_SETTINGS_FILE          "/config/lightSettings.json"

class LightSettings
{
public:
    bool softDimming = false;
    bool activeLow   = true;   // default matches the ESP32 DevKit active-low LED wiring

    static void read(LightSettings &settings, JsonObject &root)
    {
        root["soft_dimming"] = settings.softDimming;
        root["active_low"]   = settings.activeLow;
    }

    static StateUpdateResult update(JsonObject &root, LightSettings &settings, const String &originId)
    {
        bool newSoftDimming = root["soft_dimming"] | false;
        bool newActiveLow   = root["active_low"]   | true;
        if (settings.softDimming == newSoftDimming && settings.activeLow == newActiveLow)
            return StateUpdateResult::UNCHANGED;
        settings.softDimming = newSoftDimming;
        settings.activeLow   = newActiveLow;
        return StateUpdateResult::CHANGED;
    }

    static bool getSoftDimming(const LightSettings &s) { return s.softDimming; }
    static void setSoftDimming(LightSettings &s, bool v) { s.softDimming = v; }

    static bool getActiveLow(const LightSettings &s) { return s.activeLow; }
    static void setActiveLow(LightSettings &s, bool v) { s.activeLow = v; }
};

class LightSettingsService : public StatefulService<LightSettings>
{
public:
    LightSettingsService(PsychicHttpServer *server, ESP32SvelteKit *sveltekit)
        : _httpEndpoint(LightSettings::read,
                        LightSettings::update,
                        this,
                        server,
                        LIGHT_SETTINGS_REST_ENDPOINT,
                        sveltekit->getSecurityManager(),
                        AuthenticationPredicates::IS_AUTHENTICATED),
          _eventEndpoint(LightSettings::read,
                         LightSettings::update,
                         this,
                         sveltekit->getSocket(),
                         LIGHT_SETTINGS_SOCKET_EVENT),
          _fsPersistence(LightSettings::read,
                         LightSettings::update,
                         this,
                         sveltekit->getFS(),
                         LIGHT_SETTINGS_FILE),
          _haSwitches(this, sveltekit->getHAService(), "light_settings")
    {
        _haSwitches
            .addSwitch("soft_dimming","Soft Dimming",
                       LightSettings::getSoftDimming,
                       LightSettings::setSoftDimming,
                       "mdi:brightness-6",
                       HACategory::Config)
            .addSwitch("active_low", "Active Low",
                       LightSettings::getActiveLow,
                       LightSettings::setActiveLow,
                       "mdi:toggle-switch-off-outline",
                       HACategory::Config);
    }

    void begin()
    {
        _httpEndpoint.begin();
        _eventEndpoint.begin();
        _fsPersistence.readFromFS();
        _haSwitches.begin();
    }

private:
    HttpEndpoint<LightSettings>              _httpEndpoint;
    EventEndpoint<LightSettings>             _eventEndpoint;
    FSPersistence<LightSettings>             _fsPersistence;
    HAGroupedSwitchPublisher<LightSettings>  _haSwitches;
};

#endif
