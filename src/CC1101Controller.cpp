#include <CC1101Controller.h>
#include <cc1101.h>

CC1101Controller::CC1101Controller(ESP32SvelteKit *sveltekit) : _sveltekit(sveltekit),
                                                                _server(sveltekit->getServer()),
                                                                _securityManager(sveltekit->getSecurityManager())
                                                                //_eventSocket(sveltekit->getSocket()),
                                                                //_lastEmitted(0)
{
}

void CC1101Controller::begin()
{
    //_eventSocket->registerEvent(CC1101CONTROLLER_EVENT_STATE);

    //_sveltekit->addLoopFunction(std::bind(&CC1101Controller::loop, this));

    /* Register endpoints for CC1101 status */
    _server->on(CC1101CONTROLLER_SERVICE_PATH "/state",
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&CC1101Controller::_handlerGetStatus, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));
}

esp_err_t CC1101Controller::_handlerGetStatus(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject json = response.getRoot();

    uint8_t cc1101_state = 0;
    esp_err_t res = cc1101_get_state(&cc1101_state);
    json["state_success"] = (res == ESP_OK);
    if (res == ESP_OK)
        json["state"] = cc1101_state;

    return response.send();
}

// // For testing purposes
// void CC1101Controller::loop()
// {
//     uint32_t currentMillis = millis();
//     uint32_t timeElapsed = currentMillis - _lastEmitted;
//     if (currentMillis - _lastEmitted >= CC1101CONTROLLER_EVENT_PERIOD_MS)
//     {
//         _lastEmitted = currentMillis;
//         _emitCC1101State();
//     }
// }

// void CC1101Controller::_emitCC1101State()
// {
//     /* Prepare event data (payload) */
//     JsonDocument doc;
//     JsonObject json = doc.to<JsonObject>();

//     uint8_t cc1101_state = 0;
//     esp_err_t res = cc1101_get_state(&cc1101_state);
//     json["state_success"] = (res == ESP_OK);
//     if (res == ESP_OK)
//         json["state"] = cc1101_state;

//     /* Emit event */
//     _eventSocket->emitEvent(CC1101CONTROLLER_EVENT_STATE, json);
// }
