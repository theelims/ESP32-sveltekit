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

#include <HomeAssistant/HASettingsService.h>

HASettingsService::HASettingsService(PsychicHttpServer *server,
                                     FS *fs,
                                     SecurityManager *securityManager,
                                     HAService *haService)
    : _httpEndpoint(HASettings::read,
                    HASettings::update,
                    this,
                    server,
                    HA_SETTINGS_SERVICE_PATH,
                    securityManager,
                    AuthenticationPredicates::IS_ADMIN),
      _fsPersistence(HASettings::read,
                     HASettings::update,
                     this,
                     fs,
                     HA_SETTINGS_FILE),
      _haService(haService)
{
    addUpdateHandler([this](const String &originId)
                     { _applyToHAService(); },
                     false);
}

void HASettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
    _applyToHAService();
}

void HASettingsService::_applyToHAService()
{
    if (_haService == nullptr)
        return;

    bool wasReady = _haService->isReady();

    // Empty device_name falls back to APP_NAME (compile-time firmware name)
    String name = _state.deviceName.isEmpty() ? String(APP_NAME) : _state.deviceName;

    _haService->setDeviceName(name);
    _haService->setManufacturer(_state.manufacturer);
    _haService->setModel(_state.model);
    _haService->setDiscoveryPrefix(_state.discoveryPrefix);

    // Remove from HA before disabling — must happen while MQTT is still connected
    if (wasReady && !_state.enabled)
    {
        _haService->unpublishAll();
    }

    _haService->setEnabled(_state.enabled);

    ESP_LOGI(TAG, "Applied HA settings (enabled=%d, prefix=%s, device=%s)",
             _state.enabled,
             _state.discoveryPrefix.c_str(),
             name.c_str());

    // Republish on settings change if MQTT is connected
    if (_haService->isReady())
    {
        _haService->publishAll();
    }
}
