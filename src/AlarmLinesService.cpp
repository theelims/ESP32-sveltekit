#include <AlarmLinesService.h>
#include <cc1101.h>
#include <GeniusGateway.h>

/**
 * Basic Genius packet structure for the alarm line test.
 */
const uint8_t AlarmLinesService::_packet_base_linetest[] = {
    0x02,
    0xCC, 0x18, // Counter
    0x00,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0x00,
    0xFF, 0xFF, 0xFF, 0xFE, // Radio module ID, originator of the packet (0xFFFFFFFE = Gateway)
    0x00,
    0xFF, 0xFF, 0xFF, 0xFE, // Radio module ID, repeater of the packet (0xFFFFFFFE = Gateway)
    0x00, 0x00, 0x00, 0x00, // Alarm line ID
    0x0F,                   // Hops
    0x5B,
    0x48,
    0x00,
    0x66,
    0x04,
    0x04};
/**
 * Basic Genius packet structure for starting fire alarm.
 */
const uint8_t AlarmLinesService::_packet_base_firealarm[] = {
    0x02,
    0xCC, 0x18, // Counter
    0x00,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0x00,
    0xFF, 0xFF, 0xFF, 0xFE, // Radio module ID, originator of the packet (0xFFFFFFFE = Gateway)
    0x00,
    0xFF, 0xFF, 0xFF, 0xFE, // Radio module ID, repeater of the packet (0xFFFFFFFE = Gateway)
    0x00, 0x00, 0x00, 0x00, // Alarm line ID (#18-#21)
    0x0F,                   // Hops
    0x55,
    0x48,
    0x00,
    0x00,
    0x00,
    0x00, // Fire alarm start/active flag (#28)
    0x00,
    0x00, // Fire alarm end/inactive flag (#30)
    0x00,
    0xFF, 0xFF, 0xFF, 0xFE // SN of smoke detector sensing smoke (0xFFFFFFFE = Gateway)
};

AlarmLinesService::AlarmLinesService(ESP32SvelteKit *sveltekit) : _server(sveltekit->getServer()),
                                                                  _securityManager(sveltekit->getSecurityManager()),
                                                                  _featureService(sveltekit->getFeatureService()),
                                                                  _eventSocket(sveltekit->getSocket()),
                                                                  _httpEndpoint(AlarmLines::read,
                                                                                AlarmLines::update,
                                                                                this,
                                                                                sveltekit->getServer(),
                                                                                ALARMLINES_SERVICE_PATH,
                                                                                sveltekit->getSecurityManager(),
                                                                                AuthenticationPredicates::IS_ADMIN),
                                                                  _fsPersistence(AlarmLines::read,
                                                                                 AlarmLines::update,
                                                                                 this,
                                                                                 sveltekit->getFS(),
                                                                                 ALARMLINES_FILE),
                                                                  _txTaskHandle(nullptr),
                                                                  _txSemaphore(nullptr),
                                                                  _timerHandle(nullptr),
                                                                  _isTransmitting(false),
                                                                  _txRepeat(0),
                                                                  _txDataLength(0)
{
}

void AlarmLinesService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();

#if FT_ENABLED(FT_ALLOW_BROADCAST)
    _featureService->addFeature("allow_broadcast", true);
    /* Make sure, that a Broadcast alarm line is always present */
    if (!_alarmLineExists(ALARMLINES_ID_BROADCAST))
        addAlarmLine(ALARMLINES_ID_BROADCAST, "Broadcast", ALA_MANUAL, true);
#else
    _featureService->addFeature("allow_broadcast", false);
    /* Make sure, that a Broadcast alarm line is removed */
    if (_alarmLineExists(ALARMLINES_ID_BROADCAST))
        _removeAlarmLine(ALARMLINES_ID_BROADCAST);
#endif

    /* Initialize the semaphore for the TX task */
    _txSemaphore = xSemaphoreCreateBinary();
    if (_txSemaphore == NULL)
    {
        ESP_LOGE(TAG, "Failed to create TX semaphore.");
        return;
    }

    ESP_LOGI(TAG, "TX semaphore created (%p).", _txSemaphore);

    /* Create TX task */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(
        _txLoopImpl,
        ALARMLINES_TX_TASK_NAME,
        ALARMLINES_TX_TASK_STACK_SIZE,
        this,
        ALARMLINES_TX_TASK_PRIORITY,
        &_txTaskHandle,
        ALARMLINES_TX_TASK_CORE_AFFINITY);

    if (xReturned != pdPASS)
    {
        ESP_LOGE(TAG, "TX task creation failed.");
        return;
    }

    ESP_LOGI(TAG, "TX task created (%p).", _txTaskHandle);

    /* Setup timer for accurate transmission interval */
    const esp_timer_create_args_t timer_args = {
        .callback = _onTimerImpl,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = ALARMLINES_TX_PERIOD_TIMER_NAME};

    esp_err_t ret = esp_timer_create(&timer_args, &_timerHandle);
    if (ret != ESP_OK)
        ESP_LOGE(TAG, "Failed to create TX timer.");

    /* Register endpoint for triggering actions */
    _server->on(ALARMLINES_PATH_ACTIONS,
                HTTP_POST,
                _securityManager->wrapCallback(std::bind(&AlarmLinesService::_performAction, this, std::placeholders::_1, std::placeholders::_2),
                                               AuthenticationPredicates::IS_ADMIN));

    _eventSocket->registerEvent(ALARMLINES_EVENT_NEW_LINE);
}

void AlarmLinesService::_onTimer()
{
    gpio_set_level(GPIO_TEST2, 0); // Temporary for testing

    /* Notify the waiting (blocked) TX task, to start the next packet transmission iteration */
    xTaskNotifyGiveIndexed(_txTaskHandle, ALARMLINES_TX_TASK_NOTIFICATION_INDEX);
}

void AlarmLinesService::_txLoop()
{
    ESP_LOGI(pcTaskGetName(0), "Started.");

    while (1)
    {
        if (xSemaphoreTake(_txSemaphore, portMAX_DELAY) == pdTRUE)
        {
            _isTransmitting = true;

            ESP_LOGV(pcTaskGetName(0), "Starting transmission.");

            for (int i = 1; i <= _txRepeat; i++)
            {
                gpio_set_level(GPIO_TEST1, 1); // Temporary for testing
                gpio_set_level(GPIO_TEST2, 1); // Temporary for testing

                /* Resetting the timer for a single iteration */
                if (i < _txRepeat) // Don't (re)start the timer for the last iteration
                {
                    esp_err_t ret = esp_timer_start_once(_timerHandle, ALARMLINES_TX_PERIOD_MS * 1000); // 10 ms
                    if (ret != ESP_OK)
                    {
                        ESP_LOGE(TAG, "Failed to start TX timer: %s.", esp_err_to_name(ret));
                        break;
                    }
                }

                if (cc1101_send_data(_txBuffer, _txDataLength) != ESP_OK)
                    ESP_LOGE(pcTaskGetName(0), "Failed to send packet @ iteration %d.", i);

                gpio_set_level(GPIO_TEST1, 0); // Temporary for testing

                /* Wait non-blocking for the next timer period */
                if (i < _txRepeat) // Don't wait after the last iteration
                {
                    if (ulTaskNotifyTakeIndexed(ALARMLINES_TX_TASK_NOTIFICATION_INDEX, pdTRUE, ALARMLINES_TX_TASK_ITERATION_MAX_WAITING_TICKS) != 1)
                    {
                        ESP_LOGE(TAG, "Failed to receive timer notification @ iteration %d.", i);
                        break;
                    }
                }
            }

            _isTransmitting = false;

            /* Return to RX state */
            cc1101_set_rx_state();

            ESP_LOGV(pcTaskGetName(0), "Transmission finished.");
        }
        else
        {
            /* The call to ulTaskNotifyTake() timed out, if ulTaskNotifyTakeIndexed()
             * was called with xTicksToWait set to a value < portMAX_DELAY. */
            vTaskDelay(1);
        }
    }

    // never reach here
    vTaskDelete(NULL);
}

esp_err_t AlarmLinesService::_performAction(PsychicRequest *request, JsonVariant &json)
{
    if (_isTransmitting)
    {
        ESP_LOGW(TAG, "Previous action triggering is still running. Wait until it finishes to start another action.");
        return request->reply(503, "application/json", "{\"success\": false, \"reason\": \"Previous action triggering is still running.\"}");
    }

    if (!json.is<JsonObject>())
        return request->reply(400, "application/json", "{\"success\": false, \"reason\": \"Invalid JSON\"}");

    JsonObject jsonObject = json.as<JsonObject>();
    if (!jsonObject["line_id"].is<uint32_t>())
        return request->reply(400, "application/json", "{\"success\": false, \"reason\": \"Invalid line ID.\"}");

    uint32_t line_id = jsonObject["line_id"].as<uint32_t>();

    if (!jsonObject["action"].is<String>())
        return request->reply(400, "application/json", "{\"success\": false, \"reason\": \"Action missing or of wrong type.\"}");

    String action = jsonObject["action"].as<String>();

    if (action == "line-test")
    {
        size_t datalen = min(sizeof(_packet_base_linetest), sizeof(_txBuffer));
        memcpy(_txBuffer, _packet_base_linetest, datalen);
        memcpy(&_txBuffer[18], &line_id, sizeof(line_id)); // Set line_id
        _txRepeat = ALARMLINES_TX_NUM_REPEAT_LINETEST;
        _txDataLength = datalen;
    }
    else if (action == "fire-alarm-start" || action == "fire-alarm-stop")
    {
        size_t datalen = min(sizeof(_packet_base_firealarm), sizeof(_txBuffer));
        memcpy(_txBuffer, _packet_base_firealarm, min(sizeof(_packet_base_firealarm), sizeof(_txBuffer)));
        memcpy(&_txBuffer[18], &line_id, sizeof(line_id)); // Set line_id

        if (action == "firealarm-start")
            _txBuffer[28] = 0x01; // Set fire alarm start flag
        else
            _txBuffer[30] = 0x01; // Set fire alarm end flag

        _txRepeat = ALARMLINES_TX_NUM_REPEAT_FIREALARM;
        _txDataLength = datalen;
    }
    else
    {
        ESP_LOGE(TAG, "Unknown action '%s'.", action.c_str());
        return request->reply(400, "application/json", "{\"success\": false, \"reason\": \"Unknown action.\"}");
    }

    /* Notify the pending TX task to start the transmission */
    if (xSemaphoreGive(_txSemaphore) != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to give semaphore.");
        return request->reply(500, "application/json", "{\"success\": false, \"reason\": \"Failed to give semaphore.\"}");
    }

    ESP_LOGV(TAG, "Action triggered successfully für line ID '%lu'.", line_id);
    return request->reply(200, "application/json", "{\"success\": true}");
}

bool AlarmLinesService::_alarmLineExists(uint32_t id)
{
    bool found = false;

    beginTransaction();
    for (const auto &line : _state.lines)
    {
        if (line.id == id)
        {
            found = true;
            break;
        }
    }
    endTransaction();

    return found;
}

esp_err_t AlarmLinesService::addAlarmLine(uint32_t id, String name, alarm_line_acquisition_t acquisition, bool toFront)
{
    if (id == ALARMLINES_ID_NONE)
    {
        ESP_LOGE(TAG, "Cannot add a line with ID %lu. This ID is reserved.", id);
        return ESP_ERR_INVALID_ARG;
    }

    if (name.length() > ALARMLINES_NAME_MAX_LENGTH)
    {
        ESP_LOGE(TAG, "Alarm line name is too long. Maximum length is %d.", ALARMLINES_NAME_MAX_LENGTH);
        return ESP_ERR_INVALID_ARG;
    }

    if (acquisition <= ALA_MIN || acquisition >= ALA_MAX)
    {
        ESP_LOGE(TAG, "Invalid acquisition type provided: %d.", acquisition);
        return ESP_ERR_INVALID_ARG;
    }

    if (_alarmLineExists(id))
    {
        ESP_LOGW(TAG, "Alarm line with ID %lu already exists.", id);
        return ESP_ERR_INVALID_STATE;
    }

    genius_alarm_line_t newLine;
    newLine.id = id;
    newLine.name = name;
    newLine.created = time(nullptr);
    newLine.acquisition = acquisition;

    beginTransaction();
    if (toFront)
        _state.lines.insert(_state.lines.begin(), newLine);
    else
        _state.lines.push_back(newLine);
    endTransaction();

    ESP_LOGI(TAG, "Added alarm line '%s' with id %lu", newLine.name.c_str(), newLine.id);

    callUpdateHandlers(ALARMLINES_ORIGIN_ID);

    if (acquisition == ALA_GENIUS_PACKET)   // Alarm line has been added from a genius packet
        _emitNewAlarmLineEvent(id);

    return ESP_OK;
}

void AlarmLinesService::_emitNewAlarmLineEvent(uint32_t id)
{
    JsonDocument jsonDoc;
    JsonObject jsonRoot = jsonDoc.to<JsonObject>();
    jsonRoot["new_alarm_line"] = id;
    _eventSocket->emitEvent(ALARMLINES_EVENT_NEW_LINE, jsonRoot);
}

esp_err_t AlarmLinesService::_removeAlarmLine(uint32_t id)
{
    if (id == ALARMLINES_ID_NONE)
    {
        ESP_LOGE(TAG, "Cannot remove line: %lu is no valid alarm line ID.", id);
        return ESP_ERR_INVALID_ARG;
    }

    bool removed = false;

    beginTransaction();
    for (auto line = _state.lines.begin(); line != _state.lines.end(); ++line)
    {
        if (line->id == id)
        {
            _state.lines.erase(line);
            removed = true;
            break;
        }
    }
    endTransaction();

    if (!removed)
    {
        ESP_LOGW(TAG, "Alarm line with ID %lu does not exist.", id);
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGI(TAG, "Removed alarm line with id %lu", id);

    callUpdateHandlers(ALARMLINES_ORIGIN_ID);

    return ESP_OK;
}