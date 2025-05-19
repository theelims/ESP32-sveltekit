#pragma once

#include <EventSocket.h>
#include <FSPersistence.h>
#include <HttpEndpoint.h>
#include <JsonUtils.h>
#include <SecurityManager.h>
#include <PsychicHttp.h>
#include <ESP32SvelteKit.h>
#include <Utils.hpp>
#include <cc1101.h>

#define ALARMLINES_FILE "/config/alarm-lines.json"
#define ALARMLINES_SERVICE_PATH "/rest/alarm-lines"

#define ALARMLINES_PATH_ACTIONS ALARMLINES_SERVICE_PATH "/do"

#define ALARMLINES_ID_BROADCAST 0xFFFFFFFF
#define ALARMLINES_ID_NONE 0x00000000

#define ALARMLINES_MAX_NUM 100

#define ALARMLINES_NAME_MAX_LENGTH 100

#define ALARMLINES_ORIGIN_ID "alarm-lines"

#define ALARMLINES_TX_TASK_STACK_SIZE 4096
#define ALARMLINES_TX_TASK_PRIORITY 20
#define ALARMLINES_TX_TASK_NAME "alarmlines-tx"
#define ALARMLINES_TX_TASK_CORE_AFFINITY 1

#define ALARMLINES_TX_PERIOD_TIMER_NAME "alarmlines-tx-timer"
#if CORE_DEBUG_LEVEL >= 4
#define ALARMLINES_TX_PERIOD_MS 25 // 25ms
#else
#define ALARMLINES_TX_PERIOD_MS 10 // 10ms
#endif

/**
 * Index within the target task's array of task notifications to use
 * NOTE: This value muss be LESS than the value of CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES,
 * as RX_TASK_NOTIFICATION_INDEX is an 0-based index.
 */
#define ALARMLINES_TX_TASK_NOTIFICATION_INDEX 0

/**
 * Maximum ticks to wait between two packet transmission iterations.
 * NOTE: If the value is set to 'portMAX_DELAY', the task will block indefinitely.
 */
#define ALARMLINES_TX_TASK_ITERATION_MAX_WAITING_TICKS pdMS_TO_TICKS(1000) // 1 second

#define ALARMLINES_TX_NUM_REPEAT_DEFAULT 50
#define ALARMLINES_TX_NUM_REPEAT_LINETEST ALARMLINES_TX_NUM_REPEAT_DEFAULT
#define ALARMLINES_TX_NUM_REPEAT_FIREALARM ALARMLINES_TX_NUM_REPEAT_DEFAULT

#define ALARMLINES_EVENT_NEW_LINE "new-alarm-line"

typedef enum alarm_line_asquisition
{
    ALA_MIN = -1,      // Just for boundary checks
    ALA_BUILT_IN = 0,  // Alarm line added via built-in genius packet
    ALA_GENIUS_PACKET, // Alarm line added via received genius packet
    ALA_MANUAL,        // Alarm line added manually (via web interface)
    ALA_MAX            // Just for boundary checks
} alarm_line_acquisition_t;

typedef struct genius_alarm_line
{
    uint32_t id;                          // Alarm line ID (0xFFFFFFFF = broadcast, 0x00000000 = none)
    String name;                          // Alarm line name
    time_t created;                       // Creation date
    alarm_line_acquisition_t acquisition; // Acquisition type (manual or genius packet)
} genius_alarm_line_t;

class AlarmLines
{

public:
    static constexpr const char *TAG = "AlarmLines";

    std::vector<genius_alarm_line_t> lines;

    static void read(AlarmLines &alarmLines, JsonObject &root)
    {
        JsonArray jsonDevices = root["lines"].to<JsonArray>();
        for (auto &line : alarmLines.lines)
        {
            JsonObject jsonLine = jsonDevices.add<JsonObject>();
            jsonLine["id"] = line.id;
            jsonLine["name"] = line.name;
            jsonLine["created"] = Utils::time_t_to_iso8601(line.created);
            jsonLine["acquisition"] = line.acquisition;
        }

        ESP_LOGV(AlarmLines::TAG, "Alarm lines configurations read.");
    }

    static StateUpdateResult update(JsonObject &root, AlarmLines &alarmLines)
    {
        if (root["lines"].is<JsonArray>())
        {
            alarmLines.lines.clear();

            // iterate over devices
            int i = 0;
            for (JsonVariant jsonLineArrItem : root["lines"].as<JsonArray>())
            {
                if (i++ >= ALARMLINES_MAX_NUM)
                {
                    ESP_LOGE(AlarmLines::TAG, "Too many alarm lines. Maximum allowed is %d.", ALARMLINES_MAX_NUM);
                    break;
                }

                JsonObject jsonLine = jsonLineArrItem.as<JsonObject>();
                if (!jsonLine["id"].is<uint32_t>() ||
                    !jsonLine["name"].is<String>() ||
                    !jsonLine["created"].is<String>() ||
                    !jsonLine["acquisition"].is<alarm_line_acquisition_t>())
                {
                    ESP_LOGE(AlarmLines::TAG, "Invalid alarm line configuration.");
                    return StateUpdateResult::ERROR;
                }

                genius_alarm_line_t newLine;

                newLine.id = jsonLine["id"].as<uint32_t>();
                newLine.name = jsonLine["name"].as<String>();
                newLine.created = Utils::iso8601_to_time_t(jsonLine["created"].as<String>());
                newLine.acquisition = jsonLine["acquisition"].as<alarm_line_acquisition_t>();

                alarmLines.lines.push_back(newLine);

                ESP_LOGV(AlarmLines::TAG, "Added alarm line: %s", newLine.name.c_str());

                i++;
            }
        }

        ESP_LOGV(AlarmLines::TAG, "AlarmLines configurations updated.");

        return StateUpdateResult::CHANGED;
    }

private:
};

class AlarmLinesService : public StatefulService<AlarmLines>
{
public:
    static constexpr const char *TAG = "AlarmLinesService";

    AlarmLinesService(ESP32SvelteKit *sveltekit);

    void begin();

    esp_err_t addAlarmLine(uint32_t id, String name, alarm_line_acquisition_t acquisition = ALA_GENIUS_PACKET, bool toFront = false);

private:
    static const uint8_t _packet_base_linetest[];
    static const uint8_t _packet_base_firealarm[];

    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    FeaturesService *_featureService;
    EventSocket *_eventSocket;
    HttpEndpoint<AlarmLines> _httpEndpoint;
    FSPersistence<AlarmLines> _fsPersistence;

    TaskHandle_t _txTaskHandle;
    SemaphoreHandle_t _txSemaphore;
    esp_timer_handle_t _timerHandle;

    volatile bool _isTransmitting;
    uint32_t _txRepeat;
    uint8_t _txBuffer[CC1101_MAX_PACKET_LEN];
    size_t _txDataLength;

    void _txLoop();
    static void _txLoopImpl(void *_this)
    {
        static_cast<AlarmLinesService *>(_this)->_txLoop();
    }

    void _onTimer();
    static void _onTimerImpl(void *_this)
    {
        static_cast<AlarmLinesService *>(_this)->_onTimer();
    }

    bool _alarmLineExists(uint32_t id);
    esp_err_t _removeAlarmLine(uint32_t id);
    esp_err_t _performAction(PsychicRequest *request, JsonVariant &json);
    void _emitNewAlarmLineEvent(uint32_t id);

};
