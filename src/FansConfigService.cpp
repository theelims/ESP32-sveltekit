#include <FansConfigService.h>

FansConfigService::FansConfigService(ESP32SvelteKit *sveltekit) : _httpEndpoint(FansConfig::read,
                                                                                FansConfig::update,
                                                                                this,
                                                                                sveltekit->getServer(),
                                                                                FANS_CONFIG_SERVICE_PATH,
                                                                                sveltekit->getSecurityManager(),
                                                                                AuthenticationPredicates::IS_ADMIN),
                                                                  _fsPersistence(FansConfig::read,
                                                                                 FansConfig::update,
                                                                                 this,
                                                                                 sveltekit->getFS(),
                                                                                 FANS_CONFIG_FILE)
{
}

void FansConfigService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}
