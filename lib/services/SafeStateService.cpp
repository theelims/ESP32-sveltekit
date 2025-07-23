/**
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <SafeStateService.h>

SafeStateService::SafeStateService(StrokeEngine *StrokeEngine,
                                   PsychicHttpServer *server,
                                   ESP32SvelteKit *sveltekit,
                                   MqttBrokerSettingsService *mqttBrokerSettingsService) : _StrokeEngine(StrokeEngine),
                                                                                           _httpEndpoint(SafeState::read,
                                                                                                         SafeState::update,
                                                                                                         this,
                                                                                                         server,
                                                                                                         SAFE_STATE_ENDPOINT_PATH,
                                                                                                         sveltekit->getSecurityManager(),
                                                                                                         AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                           _eventEndpoint(SafeState::read,
                                                                                                          SafeState::update,
                                                                                                          this,
                                                                                                          sveltekit->getSocket(),
                                                                                                          SAFE_STATE_EVENT),
                                                                                           _mqttEndpoint(SafeState::read,
                                                                                                         SafeState::update,
                                                                                                         this,
                                                                                                         sveltekit->getMqttClient()),
                                                                                           _mqttClient(sveltekit->getMqttClient()),
                                                                                           _mqttBrokerSettingsService(mqttBrokerSettingsService),
                                                                                           _heartbeatWatchdog(STIM_FACTORY_WATCHDOG_TIMEOUT_MS)
{
    // configure settings service update handler to update safe state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(originId); },
                     false);

    _mqttBrokerSettingsService->addUpdateHandler([&](const String &originId)
                                                 { configureMqttTopics(); },
                                                 false);
}

void SafeStateService::begin()
{
    _httpEndpoint.begin();
    _eventEndpoint.begin();

    configureMqttTopics();
    _mqttEndpoint.setRetain(true);

    _state.safeState = false;

    // configure hook for watchdog
    addHookHandler([&](const String &originId, StateUpdateResult &result)
                   { 
                        // trigger watchdog if originId is not "Watchdog", "onConfigUpdated" or "StimEngine" itself
                        if (originId != "Watchdog" && originId != "onConfigUpdated" && originId != "StimEngine")
                            _heartbeatWatchdog.heartbeat(originId); },
                   false);

    _heartbeatWatchdog.onWatchdog([&](const String &originId)
                                  { watchdogTriggered(originId); });

    _StrokeEngine->onSafeState(std::bind(&SafeStateService::onSafeStateUpdated, this, std::placeholders::_1));

    onConfigUpdated("Init");
}

void SafeStateService::onConfigUpdated(String originId)
{
    if (originId == "onConfigUpdated")
    {
        ESP_LOGI(safeStateTAG, "onConfigUpdated 2nd round trip - skipping update");
        return;
    }

    ESP_LOGI(safeStateTAG, "Update Safe State: %d by %s", _state.safeState, originId.c_str());

    // update safe state in StimEngine
    if (_StrokeEngine->safeState(_state.safeState) != _state.safeState && originId != "onConfigUpdated")
    {
        // update safe state in service
        ESP_LOGI(safeStateTAG, "Forced safe state update");
        update([&](SafeState &state)
               { return StateUpdateResult::CHANGED; },
               originId);
    }
}

void SafeStateService::setHeartbeatMode(WatchdogMode mode)
{
    _heartbeatWatchdog.setWatchdogMode(mode);
}

void SafeStateService::onSafeStateUpdated(bool safestate)
{
    ESP_LOGI(safeStateTAG, "Safe state updated: %d", safestate);
    update([&](SafeState &state)
           {
               state.safeState = safestate;
               return StateUpdateResult::CHANGED; },
           "StimEngine");
}

void SafeStateService::watchdogTriggered(String originId)
{
    ESP_LOGE(safeStateTAG, "Watchdog triggered [%s] - Stopping Output", originId.c_str());
    update([&](SafeState &state)
           {
               state.safeState = true;
               return StateUpdateResult::CHANGED; },
           "Watchdog");
}

void SafeStateService::configureMqttTopics()
{
    String safeStateTopicPub;
    String safeStateTopicSub;
    _mqttBrokerSettingsService->read([&](MqttBrokerSettings &settings)
                                     { safeStateTopicPub = settings.safeStateTopicPub;
                                     safeStateTopicSub = settings.safeStateTopicSub; });
    _mqttEndpoint.configureTopics(safeStateTopicPub.c_str(), safeStateTopicSub.c_str());
}
