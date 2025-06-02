#include <FFCMqttSettingsService.h>

FFCMqttSettingsService::FFCMqttSettingsService(ESP32SvelteKit *sveltekit) : _httpEndpoint(FFCMqttSettings::read,
                                                                                          FFCMqttSettings::update,
                                                                                          this,
                                                                                          sveltekit->getServer(),
                                                                                          FFC_MQTT_SETTINGS_PATH,
                                                                                          sveltekit->getSecurityManager(),
                                                                                          AuthenticationPredicates::IS_AUTHENTICATED),
                                                                            _fsPersistence(FFCMqttSettings::read,
                                                                                           FFCMqttSettings::update,
                                                                                           this,
                                                                                           sveltekit->getFS(),
                                                                                           FFC_MQTT_SETTINGS_FILE)
{
}

void FFCMqttSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}
