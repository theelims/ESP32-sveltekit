#pragma once

#include <ESP32SvelteKit.h>
#include <EventSocket.h>
#include <AlarmService.h>
#include <driver/pulse_cnt.h>

#define RPM_SENSORS_ACQUISITION_INTERVAL_MS 2000    // 2,5 second
#define RPM_SENSORS_EVENT_ID "rpms"

#define RPM_SENSOR_GLITCH_NS 1000 // Glitch filter for PCNT unit in nanoseconds

#define RPM_NUM_FANS 2 // Number of fans (supply and exhaust)
#define RPM_INDEX_SUPPLY_FAN 0 // Index of supply fan in RPM array
#define RPM_INDEX_EXHAUST_FAN 1 // Index of exhaust fan in RPM array

class RPMSensor
{
public:
    RPMSensor(ESP32SvelteKit *sveltekit, AlarmService *alarmService, uint8_t pinSupplyFan, uint8_t pinExhaustFan);

    void begin();
    void loop();

    uint32_t getRPMSupplyFan()
    {
        return _getRPM(RPM_INDEX_SUPPLY_FAN);
    }

    uint32_t getRPMExhaustFan()
    {
        return _getRPM(RPM_INDEX_EXHAUST_FAN);
    }

private:
    static constexpr const char *TAG = "RPMSensor";

    ESP32SvelteKit *_sveltekit;
    EventSocket *_eventSocket;
    AlarmService *_alarmService;
    SemaphoreHandle_t _accessMutex;

    uint8_t _pinSupplyFan;
    uint8_t _pinExhaustFan;
    volatile uint32_t _lastLooped; // Last time (millies) the loop function was executed

    pcnt_unit_handle_t _pcnt_units[RPM_NUM_FANS];
    pcnt_channel_handle_t _pcnt_chnls[RPM_NUM_FANS];

    volatile bool _errors[RPM_NUM_FANS];

    volatile uint32_t _lastAcquired; // Last time (millies) RPMs were acquired

    uint32_t _rpms[RPM_NUM_FANS]; // RPMs of fans

    void _acquireRPMs();
    void _emitSensorValues();

    inline uint32_t _getRPM(uint8_t fanIndex)
    {
        uint32_t rpm = 0;
        
        if (fanIndex < RPM_NUM_FANS)
        {
            _beginTransaction();
            rpm = _rpms[fanIndex];
            _endTransaction();        }

        return rpm;
    }

    inline void _beginTransaction()
    {
        xSemaphoreTakeRecursive(_accessMutex, portMAX_DELAY);
    }

    inline void _endTransaction()
    {
        xSemaphoreGiveRecursive(_accessMutex);
    }
    
};