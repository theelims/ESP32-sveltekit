#pragma once

#include <ESP32SvelteKit.h>
#include <Utils.hpp>
#include <FFCMqttSettingsService.h>

#define ALARMS_SERVICE_PATH "/rest/alarms"
#define ALARMS_CONFIRM_PATH ALARMS_SERVICE_PATH "/confirm"
#define ALARMS_FILE "/config/alarms.json"

#define ALARMS_EVENT_ID "alarm"

typedef struct alarm_log_entry_
{
    uint64_t created;   // Timestamp when the alarm was created
    uint32_t id;        // Unique code for the alarm type
    String message;     // Description of the alarm event
    boolean active;     // Indicates if the alarm is currently active
    uint64_t confirmed; // Timestamp when the alarm was confirmed, 0 if not confirmed
} alarm_log_entry_t;

class AlarmLog
{

public:
    static constexpr const char *TAG = "AlarmLog";

    std::vector<alarm_log_entry_t> _entries; // Vector to hold alarm log entries

    static void read(AlarmLog &log, JsonObject &root)
    {
        JsonArray jsonLog = root["alarms"].to<JsonArray>();
        for (const auto &entry : log._entries)
        {
            JsonObject jsonEntry = jsonLog.add<JsonObject>();
            readSingleAlarm(entry, jsonEntry);
        }

        ESP_LOGV(AlarmLog::TAG, "Alarms log read.");
    }

    static StateUpdateResult update(JsonObject &root, AlarmLog &log)
    {
        log._entries.clear();

        if (root["alarms"].is<JsonArray>())
        {
            JsonArray jsonLog = root["alarms"].as<JsonArray>();
            for (JsonObject jsonEntry : jsonLog)
            {
                alarm_log_entry_t entry;
                entry.created = Utils::iso8601_to_time_t(jsonEntry["created"].as<String>());
                entry.id = jsonEntry["id"].as<uint32_t>();
                entry.message = jsonEntry["message"].as<String>();
                entry.active = jsonEntry["active"].as<bool>();
                entry.confirmed = Utils::iso8601_to_time_t(jsonEntry["confirmed"].as<String>());

                log._entries.push_back(entry);
            }
        }
        
        ESP_LOGI(AlarmLog::TAG, "Alarms log updated.");


        return StateUpdateResult::CHANGED;
    }

    static void readSingleAlarm(const alarm_log_entry_t &entry, JsonObject &root)
    {
        root["created"] = Utils::time_t_to_iso8601(entry.created);
        root["id"] = entry.id;
        root["message"] = entry.message;
        root["active"] = entry.active;
        root["confirmed"] = Utils::time_t_to_iso8601(entry.confirmed);
    }
};

class AlarmService : StatefulService<AlarmLog>
{
public:
    static constexpr const char *TAG = "AlarmService";

    AlarmService(ESP32SvelteKit *sveltekit);

    void begin();

    esp_err_t publishAlarm(const String &message);    

private:
    HttpEndpoint<AlarmLog> _httpEndpoint;
    FSPersistence<AlarmLog> _fsPersistence;
    EventSocket *_eventSocket;
    PsychicMqttClient *_mqttClient;
    FFCMqttSettingsService _ffcMqttSettingsService;

    const alarm_log_entry_t *_getAlarmById(uint32_t id);

    void _emitAlarmTCP(uint32_t id);
    void _emitAlarmMQTT(uint32_t id);

    uint32_t _generateUID(uint64_t &timestamp)
    {
        return static_cast<uint32_t>(timestamp & 0xFFFFFFFF);
    }
};
