#include "ControllerSettingsService.h"

ControllerSettingsService::ControllerSettingsService(ESP32SvelteKit *sveltekit) : _httpEndpoint(ControllerSettings::read,
                                                                                                ControllerSettings::update,
                                                                                                this,
                                                                                                sveltekit->getServer(),
                                                                                                CONTROLLER_SETTINGS_PATH,
                                                                                                sveltekit->getSecurityManager(),
                                                                                                AuthenticationPredicates::IS_ADMIN),
                                                                                  _fsPersistence(ControllerSettings::read,
                                                                                                 ControllerSettings::update,
                                                                                                 this,
                                                                                                 sveltekit->getFS(),
                                                                                                 CONTROLLER_SETTINGS_FILE)
{
}

void ControllerSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}

esp_err_t ControllerSettingsService::getCurrentSettings(controller_settings_t *settings) {
    
    if (!settings) {
        ESP_LOGE(ControllerSettings::TAG, "Invalid settings pointer provided.");
        return ESP_ERR_INVALID_ARG;
    }

    beginTransaction();
    *settings = _state.main;
    endTransaction();

    return ESP_OK;
}