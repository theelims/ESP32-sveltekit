#include <GatewayMqttSettingsService.h>

GatewayMqttSettingsService::GatewayMqttSettingsService(ESP32SvelteKit *sveltekit) : _httpEndpoint(GatewayMqttSettings::read,
                                                                                                  GatewayMqttSettings::update,
                                                                                                  this,
                                                                                                  sveltekit->getServer(),
                                                                                                  GATEWAY_MQTT_SETTINGS_PATH,
                                                                                                  sveltekit->getSecurityManager(),
                                                                                                  AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                    _fsPersistence(GatewayMqttSettings::read,
                                                                                                   GatewayMqttSettings::update,
                                                                                                   this,
                                                                                                   sveltekit->getFS(),
                                                                                                   GATEWAY_MQTT_SETTINGS_FILE)
{
}

void GatewayMqttSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}
