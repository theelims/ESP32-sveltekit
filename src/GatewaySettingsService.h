#ifndef GatewaySettingsService_h
#define GatewaySettingsService_h

#include <FSPersistence.h>
#include <HttpEndpoint.h>
#include <JsonUtils.h>
#include <SecurityManager.h>
#include <PsychicHttp.h>
#include <ESP32SvelteKit.h>
#include <Utils.hpp>

#define GATEWAY_SETTINGS_FILE "/config/gateway-settings.json"
#define GATEWAY_SETTINGS_SERVICE_PATH "/rest/gateway-settings"

#define GATEWAY_SETTINGS_STR_ALERT_ON_UNKNOWN_DETECTORS "alert_on_unknown_detectors"
#define GATEWAY_SETTINGS_STR_ADD_UNKNOWN_ALARMING_DETECTORS "add_unknown_alarming_detectors"
#define GATEWAY_SETTINGS_STR_ADD_ALARM_LINE_FROM_COMMISSIONING_PACKET "add_alarm_line_from_commissioning_packet"
#define GATEWAY_SETTINGS_STR_ADD_ALARM_LINE_FROM_ALARM_PACKET "add_alarm_line_from_alarm_packet"
#define GATEWAY_SETTINGS_STR_ADD_ALARM_LINE_FROM_LINE_TEST_PACKET "add_alarm_line_from_line_test_packet"
#define GATEWAY_SETTINGS_DEFAULT_ALERT_ON_UNKNOWN_DETECTORS true
#define GATEWAY_SETTINGS_DEFAULT_ADD_UNKNOWN_ALARMING_DETECTORS true
#define GATEWAY_SETTINGS_DEFAULT_ADD_ALARM_LINE_FROM_COMMISSIONING_PACKET true
#define GATEWAY_SETTINGS_DEFAULT_ADD_ALARM_LINE_FROM_ALARM_PACKET true
#define GATEWAY_SETTINGS_DEFAULT_ADD_ALARM_LINE_FROM_LINE_TEST_PACKET true

class GatewaySettings
{

public:
    bool alertOnUnknownDetectors = GATEWAY_SETTINGS_DEFAULT_ALERT_ON_UNKNOWN_DETECTORS;
    bool addUnknownAlarmingDetectors = GATEWAY_SETTINGS_DEFAULT_ADD_UNKNOWN_ALARMING_DETECTORS;
    bool addALarmLineFromCommissioningPacket = GATEWAY_SETTINGS_DEFAULT_ADD_ALARM_LINE_FROM_COMMISSIONING_PACKET;
    bool addAlarmLineFromAlarmPacket = GATEWAY_SETTINGS_DEFAULT_ADD_ALARM_LINE_FROM_ALARM_PACKET;
    bool addAlarmLineFromLineTestPacket = GATEWAY_SETTINGS_DEFAULT_ADD_ALARM_LINE_FROM_LINE_TEST_PACKET;

    static void read(GatewaySettings &gwSettings, JsonObject &root)
    {
        root[GATEWAY_SETTINGS_STR_ALERT_ON_UNKNOWN_DETECTORS] = gwSettings.alertOnUnknownDetectors;
        root[GATEWAY_SETTINGS_STR_ADD_UNKNOWN_ALARMING_DETECTORS] = gwSettings.addUnknownAlarmingDetectors;
        root[GATEWAY_SETTINGS_STR_ADD_ALARM_LINE_FROM_COMMISSIONING_PACKET] = gwSettings.addALarmLineFromCommissioningPacket;
        root[GATEWAY_SETTINGS_STR_ADD_ALARM_LINE_FROM_ALARM_PACKET] = gwSettings.addAlarmLineFromAlarmPacket;
        root[GATEWAY_SETTINGS_STR_ADD_ALARM_LINE_FROM_LINE_TEST_PACKET] = gwSettings.addAlarmLineFromLineTestPacket;

        ESP_LOGV(GatewaySettings::TAG, "Gateway settings read.");
    }

    static StateUpdateResult update(JsonObject &root, GatewaySettings &gwSettings)
    {
        bool updated = false;
        bool newBoolValue;

        /* alert_on_unknown_detectors */
        newBoolValue = root[GATEWAY_SETTINGS_STR_ALERT_ON_UNKNOWN_DETECTORS] | GATEWAY_SETTINGS_DEFAULT_ALERT_ON_UNKNOWN_DETECTORS;
        if (gwSettings.alertOnUnknownDetectors != newBoolValue)
        {
            gwSettings.alertOnUnknownDetectors = newBoolValue;
            updated |= true;
        }

        /* add_unknown_alarming_detectors */
        newBoolValue = root[GATEWAY_SETTINGS_STR_ADD_UNKNOWN_ALARMING_DETECTORS] | GATEWAY_SETTINGS_DEFAULT_ADD_UNKNOWN_ALARMING_DETECTORS;
        if (gwSettings.addUnknownAlarmingDetectors != newBoolValue)
        {
            gwSettings.addUnknownAlarmingDetectors = newBoolValue;
            updated |= true;
        }

        /* add_alarm_line_from_commissioning_packet */
        newBoolValue = root[GATEWAY_SETTINGS_STR_ADD_ALARM_LINE_FROM_COMMISSIONING_PACKET] | GATEWAY_SETTINGS_DEFAULT_ADD_ALARM_LINE_FROM_COMMISSIONING_PACKET;
        if (gwSettings.addALarmLineFromCommissioningPacket != newBoolValue)
        {
            gwSettings.addALarmLineFromCommissioningPacket = newBoolValue;
            updated |= true;
        }

        /* add_alarm_line_from_alarm_packet */
        newBoolValue = root[GATEWAY_SETTINGS_STR_ADD_ALARM_LINE_FROM_ALARM_PACKET] | GATEWAY_SETTINGS_DEFAULT_ADD_ALARM_LINE_FROM_ALARM_PACKET;
        if (gwSettings.addAlarmLineFromAlarmPacket != newBoolValue)
        {
            gwSettings.addAlarmLineFromAlarmPacket = newBoolValue;
            updated |= true;
        }

        /* add_alarm_line_from_line_test_packet */
        newBoolValue = root[GATEWAY_SETTINGS_STR_ADD_ALARM_LINE_FROM_LINE_TEST_PACKET] | GATEWAY_SETTINGS_DEFAULT_ADD_ALARM_LINE_FROM_LINE_TEST_PACKET;
        if (gwSettings.addAlarmLineFromLineTestPacket != newBoolValue)
        {
            gwSettings.addAlarmLineFromLineTestPacket = newBoolValue;
            updated |= true;
        }

        if (updated) {
            ESP_LOGV(GatewaySettings::TAG, "Gateway settings updated.");
            return StateUpdateResult::CHANGED;
        }
        
        return StateUpdateResult::UNCHANGED;
    }

private:
    static constexpr const char *TAG = "GatewaySettings";
};

class GatewaySettingsService : public StatefulService<GatewaySettings>
{
public:
    GatewaySettingsService(ESP32SvelteKit *sveltekit);

    void begin();

    bool isAlertOnUnknownDetectorsEnabled()
    {
        beginTransaction();
        bool val = _state.alertOnUnknownDetectors;
        endTransaction();
        return val;
    }

    bool isAddAlarmLineFromCommissioningPacketEnabled()
    {
        beginTransaction();
        bool val = _state.addALarmLineFromCommissioningPacket;
        endTransaction();
        return val;
    }

    bool isAddAlarmLineFromAlarmPacketEnabled()
    {
        beginTransaction();
        bool val = _state.addAlarmLineFromAlarmPacket;
        endTransaction();
        return val;
    }

    bool isAddAlarmLineFromLineTestPacketEnabled()
    {
        beginTransaction();
        bool val = _state.addAlarmLineFromLineTestPacket;
        endTransaction();
        return val;
    }

    bool isAddUnknownAlarmingDetectorEnabled()
    {
        beginTransaction();
        bool val = _state.addUnknownAlarmingDetectors;
        endTransaction();
        return val;
    }

private:
    HttpEndpoint<GatewaySettings> _httpEndpoint;
    FSPersistence<GatewaySettings> _fsPersistence;
};

#endif // GatewaySettingsService_h