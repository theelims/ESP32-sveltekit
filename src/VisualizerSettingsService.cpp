#include <VisualizerSettingsService.h>

VisualizerSettingsService::VisualizerSettingsService(ESP32SvelteKit *sveltekit) : _httpEndpoint(VisualizerSettings::read,
                                                                                                VisualizerSettings::update,
                                                                                                this,
                                                                                                sveltekit->getServer(),
                                                                                                PACKET_VISUALIZER_SETTINGS_SERVICE_PATH,
                                                                                                sveltekit->getSecurityManager(),
                                                                                                AuthenticationPredicates::IS_ADMIN),
                                                                                  _fsPersistence(VisualizerSettings::read,
                                                                                                 VisualizerSettings::update,
                                                                                                 this,
                                                                                                 sveltekit->getFS(),
                                                                                                 PACKET_VISUALIZER_SETTINGS_FILE)
{
}

void VisualizerSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}