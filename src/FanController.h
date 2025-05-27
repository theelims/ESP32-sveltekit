#pragma once

#include <ESP32SvelteKit.h>
#include <ControllerSettingsService.h>
#include <TempSensorsService.h>

#define CONTROLLER_STATUS_PATH "/rest/controller/status"
#define CONTROLLER_STATE_EVENT_ID "cntrstat"

#define MAX_DUTY_CYCLE 1024 // max for 10-bit resolution

#define CONTROLLER_INTERVALL_MS 5000 // ms

class FanController {
public:
    static constexpr const char *TAG = "FanController";

    FanController(ESP32SvelteKit *sveltekit);

    void begin();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    EventSocket *_eventSocket;
    ControllerSettingsService _controllerSettingsService;
    TempSensorsService _tempSensorsService;

    TaskHandle_t _txTaskHandle;

    void _ctrlLoop();
    static void _ctrlLoopImpl(void *_this) { static_cast<FanController *>(_this)->_ctrlLoop(); }

    esp_err_t _statusAsJSON(JsonObject &root);
    esp_err_t _handlerGetStatus(PsychicRequest *request);
    void _emitStatus();
};