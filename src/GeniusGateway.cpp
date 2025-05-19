#include <GeniusGateway.h>
#include <GatewaySettingsService.h>

TaskHandle_t GeniusGateway::xRxTaskHandle = NULL;

static void nofifyReceivedPacket()  // !!! This function is called from ISR !!!
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Notify the waiting (blocked) RX task that a packet is ready to be read from RX FIFO
    vTaskNotifyGiveIndexedFromISR(GeniusGateway::xRxTaskHandle,
                                  RX_TASK_NOTIFICATION_INDEX,
                                  &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch should be
       performed to ensure the interrupt returns directly to the highest priority task. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

GeniusGateway::GeniusGateway(ESP32SvelteKit *sveltekit) : _gatewayDevices(sveltekit),
                                                          _alarmLines(sveltekit),
                                                          _gatewaySettings(sveltekit),
                                                          _gatewayMqttSettingsService(sveltekit),
                                                          _mqttClient(sveltekit->getMqttClient()),
                                                          _webSocketLogger(sveltekit),
                                                          _visualizerSettingsService(sveltekit),
                                                          _cc1101Controller(sveltekit),
                                                          _eventSocket(sveltekit->getSocket()),
                                                          _featureService(sveltekit->getFeatureService())
{
}

void GeniusGateway::begin()
{
    /* TEMPORARY: Configure helper GPIO to measure packet handlig time
     * from GDO0 interrupt to full packet read */
    gpio_config_t io_conf1 = {
        .pin_bit_mask = (1ULL << GPIO_TEST1 | 1ULL << GPIO_TEST2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf1);
    gpio_set_level(GPIO_TEST1, 0);
    gpio_set_level(GPIO_TEST2, 0);
    /* END TEMPORARY */

    /* Create packet handling task */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(
        this->_rx_packetsImpl,
        RX_TASK_NAME,
        RX_TASK_STACK_SIZE,
        this,
        RX_TASK_PRIORITY,
        &GeniusGateway::xRxTaskHandle,
        RX_TASK_CORE_AFFINITY);

    if (xReturned == pdPASS)
    {
        ESP_LOGI(TAG, "RX task created (%p).", GeniusGateway::xRxTaskHandle);

        // Initialize CC1101
        esp_err_t ret = cc1101_init(nofifyReceivedPacket);
        if (ret != ESP_OK)
            ESP_LOGE(TAG, "CC1101 could not be set up.");
        else
            ESP_LOGI(TAG, "CC1101 set up successfully.");
    }
    else
    {
        ESP_LOGE(TAG, "RX task creation failed.");
    }

    /* Initialize Gateway Devices Service */
    _gatewayDevices.begin();
    /* Initialize Alarm Lines Service */
    _alarmLines.begin();
    /* Initialize Gateway Settings Service */
    _gatewaySettings.begin();
    /* Initialize Gateway MQTT Settings Service */
    _gatewayMqttSettingsService.begin();
    /* Initialize WS Logger */
    _webSocketLogger.begin();
    /* Initialize Packet Vizualizer Settings */
    _visualizerSettingsService.begin();

#if FT_ENABLED(FT_NTP)
    _featureService->addFeature("cc1101_controller", true);
    /* Initialize CC1101Controller */
    _cc1101Controller.begin();
#else
    _featureService->addFeature("cc1101_controller", false);
#endif

    /* Configure MQTT callback */
    _mqttClient->onConnect(std::bind(&GeniusGateway::_mqttPublishConfig, this));

    /* Configure update handler for when the smoke detector devices change */
    _gatewayDevices.addUpdateHandler([&](const String &originId)
                                     { _mqttPublishConfig(); },
                                     false);

    /* Configure update handler for when the MQTT settings change */
    _gatewayMqttSettingsService.addUpdateHandler([&](const String &originId)
                                                 { _mqttPublishConfig(); },
                                                 false);

    _eventSocket->registerEvent(GATEWAY_EVENT_ALARM);

    // Start the loop task
    ESP_LOGV(GeniusGateway::TAG, "Starting alarm state emitter task.");
    xTaskCreatePinnedToCore(
        this->_loopImpl,            // Function that should be called
        "alarm-state-emitter",      // Name of the task (for debugging)
        4096,                       // Stack size (bytes)
        this,                       // Pass reference to this class instance
        (tskIDLE_PRIORITY + 2),     // task priority
        NULL,                       // Task handle
        ESP32SVELTEKIT_RUNNING_CORE // Pin to application core
    );
}

// For testing purposes
void GeniusGateway::_loop()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        _emitAlarmState();
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_PERIOD_MS); // 1 seconds
    }
}

void GeniusGateway::_emitAlarmState()
{
    /* Prepare event data (payload) */
    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    root["isAlarming"] = _gatewayDevices.isAlarming();

    /* Emit event */
    _eventSocket->emitEvent(GATEWAY_EVENT_ALARM, root);
}

void GeniusGateway::_mqttPublishConfig()
{
    if (!_mqttClient->connected())
    {
        return;
    }

    GatewayMqttSettings &mqttSettings = _gatewayMqttSettingsService.getSettings(); // TODO: Not thread safe

    for (auto &device : _gatewayDevices.getDevices()) // TODO: Not thread safe
    {
        /* Publish config topic */
        String configTopic = mqttSettings.mqttPath + device.smokeDetector.sn + "/config";

        JsonDocument config_jsonDoc;
        config_jsonDoc["~"] = mqttSettings.mqttPath + device.smokeDetector.sn;
        config_jsonDoc["name"] = "Genius Plus X";
        config_jsonDoc["unique_id"] = device.smokeDetector.sn;
        config_jsonDoc["device_class"] = "smoke";
        config_jsonDoc["state_topic"] = "~/state";
        config_jsonDoc["schema"] = "json";
        config_jsonDoc["value_template"] = "{{value_json.state}}";
        config_jsonDoc["entity_picture"] = "http://genius-gateway/hekatron-genius-plus-x.png";
        JsonObject dev_jsonObj = config_jsonDoc["device"].to<JsonObject>();
        dev_jsonObj["identifiers"] = device.smokeDetector.sn;
        dev_jsonObj["manufacturer"] = "Hekatron Vertriebs GmbH";
        dev_jsonObj["model"] = "Genius Plus X";
        dev_jsonObj["name"] = "Rauchmelder";
        dev_jsonObj["serial_number"] = device.smokeDetector.sn;
        dev_jsonObj["suggested_area"] = device.location;

        String config_payload;
        serializeJson(config_jsonDoc, config_payload);
        _mqttClient->publish(configTopic.c_str(), 0, true, config_payload.c_str());

        /* Pubish state topic */
        String stateTopic = mqttSettings.mqttPath + device.smokeDetector.sn + "/state";

        JsonDocument state_jsonDoc;
        state_jsonDoc["state"] = device.isAlarming ? "ON" : "OFF";

        String payload;
        serializeJson(state_jsonDoc, payload);
        _mqttClient->publish(stateTopic.c_str(), 0, true, payload.c_str());
    }
}

esp_err_t GeniusGateway::_hekatron_analyze_packet_data(uint8_t *packet_data, size_t data_length, hekatron_packet_t *analyzed_packet)
{
    if (!packet_data)
    {
        ESP_LOGE(TAG, "%s(): Pointer to CC1101 packet data is NULL.", __func__);
        return ESP_ERR_INVALID_ARG;
    }

    if (!analyzed_packet)
    {
        ESP_LOGE(TAG, "%s(): Pointer to analyzed packet struct is NULL.", __func__);
        return ESP_ERR_INVALID_ARG;
    }

    /* Clear analyzed packet */
    memset(analyzed_packet, 0, sizeof(hekatron_packet_t));

    /* Determine type of Hekatron packet */
    switch (data_length)
    {

    case LEN_COMMISSIONING_PACKET:
        analyzed_packet->type = HPT_COMMISSIONING;
        break;
    case LEN_UNKNOWN_PURPOSE_1_PACKET:
        analyzed_packet->type = HPT_UKNOWN_PURPOSE_1;
        break;
    case LEN_UNKNOWN_PURPOSE_2_PACKET:
        analyzed_packet->type = HPT_UKNOWN_PURPOSE_2;
        break;
    case LEN_ALARM_PACKET:
        if (packet_data[DATAPOS_ALARM_ACTIVE_FLAG] == 1)
        {
            analyzed_packet->type = HPT_ALARMING;
        }
        else if (packet_data[DATAPOS_ALARM_SILENCE_FLAG] == 1)
        {
            analyzed_packet->type = HPT_ALARM_SILENCING;
        }
        else
        {
            analyzed_packet->type = HPT_UNKNOWN;
        }
        break;
    case LEN_LINE_TEST_PACKET:
        analyzed_packet->type = HPT_LINE_TEST;
        break;
    default:
        analyzed_packet->type = HPT_UNKNOWN;
        break;
    }

    if (analyzed_packet->type != HPT_UNKNOWN)
    {
        analyzed_packet->origin_id = EXTRACT32(packet_data, DATAPOS_GENERAL_ORIGIN_RADIO_MODULE_ID);
        analyzed_packet->sender_id = EXTRACT32(packet_data, DATAPOS_GENERAL_SENDER_RADIO_MODULE_ID);
        analyzed_packet->line_id = EXTRACT32(packet_data, DATAPOS_GENERAL_LINE_ID);
        analyzed_packet->hops = HOPS_FIRST - packet_data[DATAPOS_GENERAL_HOPS];
    }

    return ESP_OK;
}

void GeniusGateway::_rx_packets()
{
    cc1101_packet_t packet;
    char lineName[64];

    ESP_LOGI(pcTaskGetName(0), "Started.");

    while (1)
    {
        /* Wait (blocking) until being notified (by ISR), that a packet has been received.
           Note: the parameter 'xClearCountOnExit' is pdTRUE, which has the effect of
           clearing the task's notification value back to 0, making the notification
           value act like a binary semaphore. */
        if (ulTaskNotifyTakeIndexed(RX_TASK_NOTIFICATION_INDEX, pdTRUE, RX_TASK_MAX_WAITING_TICKS) == 1)
        {
            gpio_set_level((gpio_num_t)GPIO_TEST1, 1);  // Temporary: Measuring execution time

            // Fetch the packet
            if (cc1101_receive_data(&packet) == ESP_OK)
            {
                hekatron_packet_t packet_details;
                if (_hekatron_analyze_packet_data(packet.data, packet.length, &packet_details) == ESP_OK)
                {
                    if (packet_details.type == HPT_COMMISSIONING)
                    {
                        /* Store new alarm line id */
                        if (_gatewaySettings.isAddAlarmLineFromCommissioningPacketEnabled())
                        {
                            uint32_t newLineID = EXTRACT32(packet.data, DATAPOS_COMISSIONING_NEW_LINE_ID);
                            snprintf(lineName, sizeof(lineName), "Added from received comissioning packet", newLineID);
                            _alarmLines.addAlarmLine(newLineID, String(lineName), ALA_GENIUS_PACKET);
                        }
                    }
                    else if (packet_details.type == HPT_ALARMING || packet_details.type == HPT_ALARM_SILENCING)
                    {

                        uint32_t source_id = EXTRACT32_REV(packet.data, DATAPOS_ALARM_SOURCE_SMOKE_ALARM_ID);

                        if (packet_details.type == HPT_ALARMING)
                        {
                            bool isDetectorKnown = _gatewayDevices.isSmokeDetectorKnown(source_id);

                            if (!isDetectorKnown && _gatewaySettings.isAddUnknownAlarmingDetectorEnabled())
                            {
                                // TODO: Add unknown smoke detector to the list
                            }

                            /* Set/Reset alarm */
                            if (isDetectorKnown || (!isDetectorKnown && _gatewaySettings.isAlertOnUnknownDetectorsEnabled()))
                            {
                                _gatewayDevices.setAlarm(source_id);
                            }
                        }
                        else
                        {
                            // packet_details.type == HPT_ALARM_SILENCING
                            _gatewayDevices.resetAlarm(source_id, HAE_BY_SMOKE_DETECTOR);
                        }

                        /* Store alarm line id */
                        if (_gatewaySettings.isAddAlarmLineFromAlarmPacketEnabled())
                        {
                            uint32_t lineID = EXTRACT32(packet.data, DATAPOS_GENERAL_LINE_ID);
                            snprintf(lineName, sizeof(lineName), "Added from received alarming/silencing packet", lineID);
                            _alarmLines.addAlarmLine(lineID, String(lineName), ALA_GENIUS_PACKET);
                        }
                    }
                    else if (packet_details.type == HPT_LINE_TEST)
                    {
                        if (_gatewaySettings.isAddAlarmLineFromLineTestPacketEnabled())
                        {
                            uint32_t lineID = EXTRACT32(packet.data, DATAPOS_GENERAL_LINE_ID);
                            snprintf(lineName, sizeof(lineName), "Added from received line test packet", lineID);
                            _alarmLines.addAlarmLine(lineID, String(lineName), ALA_GENIUS_PACKET);
                        }
                    }
                }

                /* Send data to WebSocket logger */
                gpio_set_level(GPIO_TEST2, 1); // Temporary: Measuring execution time
                _webSocketLogger.logPacket(&packet);
                gpio_set_level(GPIO_TEST2, 0); // Temporary: Measuring execution time
            }

            gpio_set_level(GPIO_TEST1, 0); // Temporary: Measuring execution time
        }
        else
        {
            /* The call to ulTaskNotifyTake() timed out, if ulTaskNotifyTakeIndexed()
             * was called with xTicksToWait set to a value < portMAX_DELAY. */
            vTaskDelay(1);
        }

        /* Check for RX overflow before returning to receive state, as packet
         * handling might have taken too long to fetch next packet in time */
        cc1101_check_rx();
    }

    // never reach here
    vTaskDelete(NULL);
}