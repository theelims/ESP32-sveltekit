#include <GatewaySettingsService.h>

GatewaySettingsService::GatewaySettingsService(ESP32SvelteKit *sveltekit) : _httpEndpoint(GatewaySettings::read,
                                                                                          GatewaySettings::update,
                                                                                          this,
                                                                                          sveltekit->getServer(),
                                                                                          GATEWAY_SETTINGS_SERVICE_PATH,
                                                                                          sveltekit->getSecurityManager(),
                                                                                          AuthenticationPredicates::IS_ADMIN),
                                                                            _fsPersistence(GatewaySettings::read,
                                                                                           GatewaySettings::update,
                                                                                           this,
                                                                                           sveltekit->getFS(),
                                                                                           GATEWAY_SETTINGS_FILE)
{
}

void GatewaySettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}