/*
https://github.com/junkfix/esp32-ds18b20
*/

#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define MAX_BLOCKS	64
#else
#define MAX_BLOCKS	48
#endif

#define OWR_OK	0
#define OWR_CRC	1
#define OWR_BAD_DATA	2
#define OWR_TIMEOUT	3
#define OWR_DRIVER	4

#define OW_RESET_PULSE	500
#define OW_RESET_WAIT	200
#define OW_RESET_PRESENCE_WAIT_MIN	15
#define OW_RESET_PRESENCE_MIN	60
#define OW_SLOT_BIT_SAMPLE_TIME	15
#define OW_SLOT_START	2
#define OW_SLOT_BIT	60
#define OW_SLOT_RECOVERY	5
#define OW_TIMEOUT	50

IRAM_ATTR bool owrxdone(rmt_channel_handle_t ch, const rmt_rx_done_event_data_t *edata, void *udata);

class OneWire32 {
	private:
		gpio_num_t owpin;
		rmt_channel_handle_t owtx;
		rmt_channel_handle_t owrx;
		rmt_encoder_handle_t owcenc;
		rmt_encoder_handle_t owbenc;
		rmt_symbol_word_t owbuf[MAX_BLOCKS];
		QueueHandle_t owqueue;
		uint8_t drv = 0;
	public:
		OneWire32(uint8_t pin);
		~OneWire32();
		bool reset();
		void request();
		uint8_t getTemp(uint64_t &addr, float &temp);
		uint8_t search(uint64_t *addresses, uint8_t total);
		bool read(uint8_t &data, uint8_t len = 8);
		bool write(const uint8_t data, uint8_t len = 8);
};
