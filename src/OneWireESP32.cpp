/*
https://github.com/junkfix/esp32-ds18b20
*/

#include <Arduino.h>
#include "OneWireESP32.h"

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


static rmt_symbol_word_t ow_bit0 = {
	.duration0 = OW_SLOT_START + OW_SLOT_BIT,
	.level0 = 0,
	.duration1 = OW_SLOT_RECOVERY,
	.level1 = 1
};

static rmt_symbol_word_t ow_bit1 = {
	.duration0 = OW_SLOT_START,
	.level0 = 0,
	.duration1 = OW_SLOT_BIT + OW_SLOT_RECOVERY,
	.level1 = 1
};

const rmt_transmit_config_t owtxconf = {
	.loop_count = 0,
	.flags = {
		.eot_level = 1
	}
};
const rmt_receive_config_t owrxconf = {
	.signal_range_min_ns = 1000,
	.signal_range_max_ns = (OW_RESET_PULSE + OW_RESET_WAIT) * 1000,
};


OneWire32::OneWire32(uint8_t pin){
	owpin = static_cast<gpio_num_t>(pin);
	
	rmt_bytes_encoder_config_t bnc = {
		.bit0 = ow_bit0,
		.bit1 = ow_bit1,
		.flags = {
			.msb_first = 0
		}
	};

	if(rmt_new_bytes_encoder(&bnc, &(owbenc)) != ESP_OK) {
		return;
	}

	rmt_copy_encoder_config_t cnc = {};

	if(rmt_new_copy_encoder(&cnc, &(owcenc)) != ESP_OK) {
		return;
	}
	
	const rmt_rx_channel_config_t rxconf = {
		.gpio_num = owpin,
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.resolution_hz = 1000000,
		.mem_block_symbols = MAX_BLOCKS
	};

	if(rmt_new_rx_channel(&rxconf, &(owrx)) != ESP_OK) {
		return;
	}

	const rmt_tx_channel_config_t txconf = {
		.gpio_num = owpin,
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.resolution_hz = 1000000,
		.mem_block_symbols = MAX_BLOCKS,
		.trans_queue_depth = 4,
		.flags = {
			.io_loop_back = 1,
			.io_od_mode = 1
		}
	};

	if(rmt_new_tx_channel(&txconf, &owtx) != ESP_OK) {
		return;
	}

	owqueue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
	if(owqueue == NULL) {
		return;
	}

	rmt_rx_event_callbacks_t rx_callbacks = {
		.on_recv_done = owrxdone
	};

	if(rmt_rx_register_event_callbacks(owrx, &rx_callbacks, owqueue) != ESP_OK) {
		return;
	}

	if(rmt_enable(owrx) != ESP_OK) {
		return;
	}
	
	if(rmt_enable(owtx) != ESP_OK) {
		return;
	}

	static rmt_symbol_word_t release_symbol = {
		.duration0 = 1,
		.level0 = 1,
		.duration1 = 0,
		.level1 = 1,
	};
	rmt_transmit(owtx, owcenc, &release_symbol, sizeof(rmt_symbol_word_t), &owtxconf);

	drv = 1;
	
}


OneWire32::~OneWire32(){
	if(owbenc) {
		rmt_del_encoder(owbenc);
	}
	if(owcenc) {
		rmt_del_encoder(owcenc);
	}
	if(owrx) {
		rmt_disable(owrx);
		rmt_del_channel(owrx);
	}
	if(owtx) {
		rmt_disable(owtx);
		rmt_del_channel(owtx);
	}
	if(owqueue) {
		vQueueDelete(owqueue);
	}
	drv = 0;
}


bool owrxdone(rmt_channel_handle_t ch, const rmt_rx_done_event_data_t *edata, void *udata) {
	BaseType_t h = pdFALSE;
	xQueueSendFromISR((QueueHandle_t)udata, edata, &h);
	return (h == pdTRUE);
}


bool OneWire32::reset(){
	
	rmt_symbol_word_t symbol_reset = {
		.duration0 = OW_RESET_PULSE,
		.level0 = 0,
		.duration1 = OW_RESET_WAIT,
		.level1 = 1
	};

	rmt_rx_done_event_data_t evt;
	rmt_receive(owrx, owbuf, sizeof(owbuf), &owrxconf);
	rmt_transmit(owtx, owcenc, &symbol_reset, sizeof(rmt_symbol_word_t), &owtxconf);
	bool found = false;
	if(xQueueReceive(owqueue, &evt, pdMS_TO_TICKS(OW_TIMEOUT)) == pdTRUE) {
		size_t symbol_num = evt.num_symbols;
		rmt_symbol_word_t *symbols = evt.received_symbols;

		if (symbol_num > 1) {
			if (symbols[0].level1 == 1) {
				if (symbols[0].duration1 > OW_RESET_PRESENCE_WAIT_MIN && symbols[1].duration0 > OW_RESET_PRESENCE_MIN) {
					found = true;
				}
			} else {
				if (symbols[0].duration0 > OW_RESET_PRESENCE_WAIT_MIN && symbols[1].duration1 > OW_RESET_PRESENCE_MIN) {
					found = true;
				}
			}
		}		
		if(rmt_tx_wait_all_done(owtx, OW_TIMEOUT) != ESP_OK) {
			found = false;
		}
		
	}
	return found;	

}


bool OneWire32::read(uint8_t &data, uint8_t len){

	rmt_rx_done_event_data_t evt;
	rmt_receive(owrx, owbuf, sizeof(owbuf), &owrxconf);

	if(!write((len > 1)? 0xff : 1, len) || xQueueReceive(owqueue, &evt, pdMS_TO_TICKS(OW_TIMEOUT)) != pdTRUE) {
		return false;
	}

	size_t symbol_num = evt.num_symbols;
	rmt_symbol_word_t *symbol = evt.received_symbols;
	data = 0;
	for (uint8_t i = 0; i < symbol_num && i < 8; i++) {
		if(!(symbol[i].duration0 > OW_SLOT_BIT_SAMPLE_TIME)){
			data |= 1 << i;
		}
	}

	if(len != 8){ data = data & 0x01; }
	return true;
}


bool OneWire32::write(const uint8_t data, uint8_t len){
	
	if(len < 8){
		const rmt_symbol_word_t *sb;
		for(uint8_t i = 0; i < len; i++){
			sb = &ow_bit0;
			if((data & (1 << i)) != 0) {
				sb = &ow_bit1;
			}
			if(rmt_transmit(owtx, owcenc, sb, sizeof(rmt_symbol_word_t), &owtxconf) != ESP_OK){
				return false;
			}
		}
	}else{
		if(rmt_transmit(owtx, owbenc, &data, 1, &owtxconf) != ESP_OK){
			return false;
		}
	}

	return (rmt_tx_wait_all_done(owtx, OW_TIMEOUT) == ESP_OK);
}


void OneWire32::request(){
	if(drv && reset()){
		write(0xCC);
		write(0x44);
	}
}

const uint8_t crc_table[] = {0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65, 157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220, 35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98, 190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255, 70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7, 219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154, 101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36, 248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185, 140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205, 17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80, 175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238, 50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115, 202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139, 87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22, 233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168, 116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53};

uint8_t OneWire32::getTemp(uint64_t &addr, float &temp){
	uint8_t error = OWR_OK;
	if(!drv){return OWR_DRIVER;}
	if(reset()){	//connected
		write(0x55);
		uint8_t *a = (uint8_t *)&addr;
		for(uint8_t i = 0; i < 8; i++){
			write(a[i]);
		}
		write(0xBE);	// Read
		uint8_t data[9]; uint16_t zero = 0; uint8_t crc = 0;
		for(uint8_t j = 0; j < 9; j++){
			if(!read(data[j],8)){data[j] = 0;}
			zero += data[j];
			if(j < 8 ){crc = crc_table[crc ^ data[j]];}
		}
		if(zero != 0x8f7 && zero){
			if(data[8] == crc ){	//CRC OK
				int16_t t = (data[1] << 8) | data[0];
				temp = ((float)t / 16.0);
			}else{
				error = OWR_CRC;
			}
		}else{
			error = OWR_BAD_DATA;
		}
	}else{
		error = OWR_TIMEOUT;
	}
	return error;
}


uint8_t OneWire32::search(uint64_t *addresses, uint8_t total) {
	int8_t last_src;
	int8_t last_dev = -1;
	uint8_t found = 0;
	uint8_t loop = 1;
	if(!drv){return found;}
	uint64_t addr = 0;
	while(loop && found < total){
		loop = 0;
		last_src = last_dev;
		if(!reset()){
			found = 0;
			break;
		}
		write(0xF0,8);
		for(uint8_t i = 0; i < 64; i += 1){
			uint8_t bitA, bitB; uint64_t m = 1ULL << i;
			if(!read(bitA, 1) || !read(bitB, 1) || (bitA && bitB)){
				addr = found = loop = 0;
				break;
			}else if(!bitA && !bitB){
				if(i == last_src){
					write(1, 1); addr |= m;
				}else{
					if((addr & m) == 0 || i > last_src){
						write(0, 1); loop = 1; addr &= ~m;
						last_dev = i;
					}else{
						write(1, 1);
					}
				}
			}else{
				if(bitA){
					write(1, 1); addr |= m;
				}else{
					write(0, 1); addr &= ~m;
				}
			}
		}
		if(addr){
			addresses[found] = addr;
			found++;
		}
	}
	return found;
}
