#include "AlarmService.h"

AlarmService::AlarmService(ESP32SvelteKit *sveltekit) : _httpEndpoint(AlarmLog::read,
                                                                      AlarmLog::update,
                                                                      this,
                                                                      sveltekit->getServer(),
                                                                      ALARMS_SERVICE_PATH,
                                                                      sveltekit->getSecurityManager(),
                                                                      AuthenticationPredicates::IS_ADMIN),
                                                        _fsPersistence(AlarmLog::read,
                                                                       AlarmLog::update,
                                                                       this,
                                                                       sveltekit->getFS(),
                                                                       ALARMS_FILE),
                                                        _eventSocket(sveltekit->getSocket()),
                                                        _mqttClient(sveltekit->getMqttClient()),
                                                        _ffcMqttSettingsService(sveltekit)
{
}

void AlarmService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();

    _ffcMqttSettingsService.begin();

    _eventSocket->registerEvent(ALARMS_EVENT_ID);
}

esp_err_t AlarmService::publishAlarm(const String &message)
{
    alarm_log_entry_t entry;
    entry.created = time(nullptr);
    entry.id = _generateUID(entry.created);
    entry.message = message;
    entry.active = true;
    entry.confirmed = 0;

    beginTransaction();
    _state._entries.insert(_state._entries.begin(), entry);
    endTransaction();

    _emitAlarmTCP(entry.id); // Publish via HTTP/WebSocket
    _emitAlarmMQTT(entry.id); // Publish via MQTT

    return ESP_OK;
}

const alarm_log_entry_t *AlarmService::_getAlarmById(uint32_t id)
{
    const alarm_log_entry_t *p = nullptr;
    beginTransaction();
    for (const auto &entry : _state._entries)
    {
        if (entry.id == id)
        {
            p = &entry;
            break;
        }
    }
    endTransaction();
    return p;
}

void AlarmService::_emitAlarmTCP(uint32_t id)
{
    const alarm_log_entry_t *alarm = _getAlarmById(id);
    if (!alarm)
    {
        return;
    }

    JsonDocument alarm_jsonDoc;
    JsonObject alarm_jsonRoot = alarm_jsonDoc.to<JsonObject>();

    beginTransaction();
    AlarmLog::readSingleAlarm(*alarm, alarm_jsonRoot);
    endTransaction();

    /* Web Socket (to frontend) */
    _eventSocket->emitEvent(ALARMS_EVENT_ID, alarm_jsonRoot);
    /* HTTP POST request (smarthome integration) */
    // ...
}

void AlarmService::_emitAlarmMQTT(uint32_t id)
{
    if (!_mqttClient->connected())
    {
        return;
    }

    const alarm_log_entry_t *alarm = _getAlarmById(id);
    if (!alarm)
    {
        return;
    }

    String alarmTopic = _ffcMqttSettingsService.getTopic();
    JsonDocument alarm_jsonDoc;
    JsonObject alarm_jsonRoot = alarm_jsonDoc.to<JsonObject>();

    beginTransaction();
    AlarmLog::readSingleAlarm(*alarm, alarm_jsonRoot);
    endTransaction();

    String payload;
    serializeJson(alarm_jsonDoc, payload);
    _mqttClient->publish(alarmTopic.c_str(), 0, false, payload.c_str());
}
