#pragma once

#include <ESP32SvelteKit.h>
#include <ControllerSettingsService.h>
#include <FansConfigService.h>
#include <TempSensorsService.h>
#include <AlarmService.h>
#include <RPMSensor.h>

#define CONTROLLER_STATE_PATH "/rest/controller/state"
#define CONTROLLER_STATE_EVENT_ID "ctrl-state"

#define MAX_DUTY_CYCLE 1024 // max for 10-bit resolution

#define CONTROLLER_INTERVALL_MS 5000 // ms

#define CONTROLLER_TEMP_MONITOR_HYSTERESIS 2.5f // °C

#define CONTROLLER_MIN_RPM 400
#define CONTROLLER_MIN_RPM_SUPPLY_FAN CONTROLLER_MIN_RPM
#define CONTROLLER_MIN_RPM_EXHAUST_FAN CONTROLLER_MIN_RPM

typedef struct controller_state {
    float baseTemp;
    uint32_t dutyCycle;
    uint32_t fan1RPM;
    uint32_t fan2RPM;
} controller_state_t;

class FanController {
public:
    static constexpr const char *TAG = "FanController";

    FanController(ESP32SvelteKit *sveltekit);

    void begin();
    void loop();

private:
    ESP32SvelteKit *_sveltekit;
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    EventSocket *_eventSocket;
    ControllerSettingsService _controllerSettingsService;
    AlarmService _alarmService;
    FansConfigService _fansConfigService;
    TempSensorsService _tempSensorsService;
    RPMSensor _rpmSensor;
    SemaphoreHandle_t _accessMutex;

    volatile uint32_t _lastAcquired; // Last time (millies) the controller was executed

    bool _tempError;
    bool _fanError;
    volatile controller_state_t _state;

    void _ctrlLoop();
    static void _ctrlLoopImpl(void *_this) { static_cast<FanController *>(_this)->_ctrlLoop(); }

    esp_err_t _stateAsJSON(JsonObject &root);
    esp_err_t _handlerGetState(PsychicRequest *request);
    void _emitState();

    inline void _beginTransaction()
    {
        xSemaphoreTakeRecursive(_accessMutex, portMAX_DELAY);
    }

    inline void _endTransaction()
    {
        xSemaphoreGiveRecursive(_accessMutex);
    }
};