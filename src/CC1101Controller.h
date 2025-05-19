#pragma once

#include <ESP32SvelteKit.h>
#include <EventSocket.h>
#include <FSPersistence.h>
#include <HttpEndpoint.h>
#include <JsonUtils.h>
#include <SecurityManager.h>
#include <PsychicHttp.h>
#include <Utils.hpp>
#include <cc1101.h>

#define CC1101CONTROLLER_SERVICE_PATH "/rest/cc1101"
// #define CC1101CONTROLLER_EVENT_STATE "cc1101-state"
// #define CC1101CONTROLLER_EVENT_PERIOD_MS 10000 // 10 seconds

class CC1101Controller
{
public:
    static constexpr const char *TAG = "CC1101Controller";

    CC1101Controller(ESP32SvelteKit *sveltekit);

    void begin();
    void loop();

private:
    ESP32SvelteKit *_sveltekit;
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    // EventSocket *_eventSocket;

    // volatile uint32_t _lastEmitted; // Last time (millies) the state was emitted

    // void _emitCC1101State();

    esp_err_t _handlerGetStatus(PsychicRequest *request);
};
