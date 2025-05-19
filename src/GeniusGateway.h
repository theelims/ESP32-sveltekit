#pragma once

#include <ESP32SvelteKit.h>
#include <WebSocketLogger.h>
#include <VisualizerSettingsService.h>
#include <GatewayDevicesService.h>
#include <AlarmLinesService.h>
#include <GatewaySettingsService.h>
#include <GatewayMqttSettingsService.h>
#include <CC1101Controller.h>
#include <cc1101.h>

// GPIO to use for testing purposes
#define GPIO_TEST1 GPIO_NUM_21
#define GPIO_TEST2 GPIO_NUM_14

#define RX_TASK_STACK_SIZE 4096
#define RX_TASK_PRIORITY 20
#define RX_TASK_CORE_AFFINITY 1
#define RX_TASK_NAME "hekatron-rx"

#define HOPS_FIRST 0xF
#define HOPS_LAST 0x0

#define MIN_HEKATRON_PACKET_LENGTH LEN_UNKNOWN_PURPOSE_1_PACKET

#define LEN_COMMISSIONING_PACKET 37
#define LEN_UNKNOWN_PURPOSE_1_PACKET 28
#define LEN_UNKNOWN_PURPOSE_2_PACKET 32
#define LEN_ALARM_PACKET 36
#define LEN_LINE_TEST_PACKET 29

#define DATAPOS_GENERAL_ORIGIN_RADIO_MODULE_ID 9
#define DATAPOS_GENERAL_SENDER_RADIO_MODULE_ID 14
#define DATAPOS_GENERAL_LINE_ID 18
#define DATAPOS_GENERAL_HOPS 22
#define DATAPOS_COMISSIONING_NEW_LINE_ID 28
#define DATAPOS_COMOSSIONING_TIME_HOUR 32
#define DATAPOS_COMOSSIONING_TIME_MINUTE 33
#define DATAPOS_COMOSSIONING_TIME_SECOND 34
#define DATAPOS_ALARM_ACTIVE_FLAG 28
#define DATAPOS_ALARM_SILENCE_FLAG 30
#define DATAPOS_ALARM_SOURCE_SMOKE_ALARM_ID 32

#define EXTRACT32(buffer, pos) (__ntohl(*(uint32_t *)&buffer[pos]))
#define EXTRACT32_REV(buffer, pos) (*(uint32_t *)&buffer[pos])

/**
 * Index within the target task's array of task notifications to use
 * NOTE: This value muss be LESS than the value of CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES,
 * as RX_TASK_NOTIFICATION_INDEX is an 0-based index.
 */
#define RX_TASK_NOTIFICATION_INDEX 0

/**
 * Maximum ticks to wait for a packet to be received, before the blocking
 * task is unblocked to do some further checks or error handling.
 * NOTE: If the value is set to 'portMAX_DELAY', the task will block indefinitely.
 */
#define RX_TASK_MAX_WAITING_TICKS portMAX_DELAY

#define GATEWAY_EVENT_ALARM "alarm"

typedef enum hekatron_packet_type
{
  HPT_UNKNOWN = 0,
  HPT_COMMISSIONING,
  HPT_UKNOWN_PURPOSE_1,
  HPT_UKNOWN_PURPOSE_2,
  HPT_ALARMING,
  HPT_ALARM_SILENCING,
  HPT_LINE_TEST
} hekatron_packet_type_t;

typedef struct hekatron_packet_t
{
  hekatron_packet_type_t type;
  uint32_t origin_id;
  uint32_t sender_id;
  uint32_t line_id;
  uint8_t hops;
} hekatron_packet_t;

class GeniusGateway
{
public:
  GeniusGateway(ESP32SvelteKit *sveltekit);

  static TaskHandle_t xRxTaskHandle;

  void begin();

private:
  static constexpr const char *TAG = "GeniusGateway";

  GatewayDevicesService _gatewayDevices;
  AlarmLinesService _alarmLines;
  GatewaySettingsService _gatewaySettings;
  GatewayMqttSettingsService _gatewayMqttSettingsService;
  WebSocketLogger _webSocketLogger;
  VisualizerSettingsService _visualizerSettingsService;
  CC1101Controller _cc1101Controller;
  PsychicMqttClient *_mqttClient;
  EventSocket *_eventSocket;
  FeaturesService *_featureService;

  void _mqttPublishConfig();

  void _rx_packets();
  static void _rx_packetsImpl(void *_this) { static_cast<GeniusGateway *>(_this)->_rx_packets(); }

  /**
   * @brief Analyze a received packet
   *
   * Analyze a received packet and fill the analyzed_packet struct with the results.
   *
   * @param packet_data Pointer to the received packet buffer
   * @param data_length Length of the received packet
   * @param analyzed_packet Pointer to the analyzed packet
   */
  esp_err_t _hekatron_analyze_packet_data(uint8_t *packet_data, size_t data_length, hekatron_packet_t *analyzed_packet);


    void _loop();
    static void _loopImpl(void *_this) { static_cast<GeniusGateway *>(_this)->_loop(); }

    void _emitAlarmState();

};
