#pragma once

#include <ESP32SvelteKit.h>
#include <EventSocket.h>
#include <driver/pulse_cnt.h>

#define RPM_SENSORS_ACQUISITION_INTERVAL_MS 2000    // 2,5 second
#define RPM_SENSORS_EVENT_ID "rpms"

#define RPM_SENSOR_GLITCH_NS 1000 // Glitch filter for PCNT unit in nanoseconds

class RPMSensor
{
public:
    RPMSensor(ESP32SvelteKit *sveltekit, uint8_t pinSupplyFan, uint8_t pinExhaustFan);

    void begin();
    void loop();

private:
    static constexpr const char *TAG = "RPMSensor";

    ESP32SvelteKit *_sveltekit;
    EventSocket *_eventSocket;
    SemaphoreHandle_t _accessMutex;

    uint8_t _pinSupplyFan;
    uint8_t _pinExhaustFan;
    volatile uint32_t _lastLooped; // Last time (millies) the loop function was executed

    pcnt_unit_handle_t _pcnt_unit;
    pcnt_channel_handle_t _pcnt_chan_a;

    volatile uint32_t _lastAcquired; // Last time (millies) RPMs were acquired

    uint32_t _rpm_supplyFan;
    uint32_t _rpm_exhaustFan;

    void _acquireRPMs();
    void _emitSensorValues();

    inline void _beginTransaction()
    {
        xSemaphoreTakeRecursive(_accessMutex, portMAX_DELAY);
    }

    inline void _endTransaction()
    {
        xSemaphoreGiveRecursive(_accessMutex);
    }
    
};