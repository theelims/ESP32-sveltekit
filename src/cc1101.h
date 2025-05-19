#pragma once

#include <driver/spi_master.h>
#include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type of transfers
 */
#define WRITE_BURST 0x40
#define READ_SINGLE 0x80
#define READ_BURST  0xC0

/**
 * Type of register
 */
#define CC1101_CONFIG_REGISTER	 READ_SINGLE
#define CC1101_STATUS_REGISTER	 READ_BURST

/**
 * PATABLE & FIFO's
 */
#define CC1101_PATABLE				0x3E	// PATABLE address
#define CC1101_TXFIFO				0x3F	// TX FIFO address
#define CC1101_RXFIFO				0x3F	// RX FIFO address

/**
 * Command strobes
 */
#define CC1101_SRES					0x30	// Reset CC1101 chip
#define CC1101_SFSTXON				0x31	// Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX (with CCA):
											// Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
#define CC1101_SXOFF				0x32	// Turn off crystal oscillator
#define CC1101_SCAL					0x33	// Calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without
											// setting manual calibration mode (MCSM0.FS_AUTOCAL=0)
#define CC1101_SRX					0x34	// Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1
#define CC1101_STX					0x35	// In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1.
											// If in RX state and CCA is enabled: Only go to TX if channel is clear
#define CC1101_SIDLE				0x36	// Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable
#define CC1101_SWOR					0x38	// Start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if
											// WORCTRL.RC_PD=0
#define CC1101_SPWD					0x39	// Enter power down mode when CSn goes high
#define CC1101_SFRX					0x3A	// Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states
#define CC1101_SFTX					0x3B	// Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states
#define CC1101_SWORRST				0x3C	// Reset real time clock to Event1 value
#define CC1101_SNOP					0x3D	// No operation. May be used to get access to the chip status byte

/**
 * CC1101 configuration registers
 */
#define CC1101_IOCFG2				0x00	// GDO2 Output Pin Configuration
#define CC1101_IOCFG1				0x01	// GDO1 Output Pin Configuration
#define CC1101_IOCFG0				0x02	// GDO0 Output Pin Configuration
#define CC1101_FIFOTHR				0x03	// RX FIFO and TX FIFO Thresholds
#define CC1101_SYNC1				0x04	// Sync Word, High Byte
#define CC1101_SYNC0				0x05	// Sync Word, Low Byte
#define CC1101_PKTLEN				0x06	// Packet Length
#define CC1101_PKTCTRL1				0x07	// Packet Automation Control
#define CC1101_PKTCTRL0				0x08	// Packet Automation Control
#define CC1101_ADDR					0x09	// Device Address
#define CC1101_CHANNR				0x0A	// Channel Number
#define CC1101_FSCTRL1				0x0B	// Frequency Synthesizer Control
#define CC1101_FSCTRL0				0x0C	// Frequency Synthesizer Control
#define CC1101_FREQ2			 	0x0D	// Frequency Control Word, High Byte
#define CC1101_FREQ1				0x0E	// Frequency Control Word, Middle Byte
#define CC1101_FREQ0				0x0F	// Frequency Control Word, Low Byte
#define CC1101_MDMCFG4				0x10	// Modem Configuration
#define CC1101_MDMCFG3				0x11	// Modem Configuration
#define CC1101_MDMCFG2				0x12	// Modem Configuration
#define CC1101_MDMCFG1				0x13	// Modem Configuration
#define CC1101_MDMCFG0				0x14	// Modem Configuration
#define CC1101_DEVIATN				0x15	// Modem Deviation Setting
#define CC1101_MCSM2				0x16	// Main Radio Control State Machine Configuration
#define CC1101_MCSM1				0x17	// Main Radio Control State Machine Configuration
#define CC1101_MCSM0				0x18	// Main Radio Control State Machine Configuration
#define CC1101_FOCCFG				0x19	// Frequency Offset Compensation Configuration
#define CC1101_BSCFG				0x1A	// Bit Synchronization Configuration
#define CC1101_AGCCTRL2				0x1B	// AGC Control
#define CC1101_AGCCTRL1				0x1C	// AGC Control
#define CC1101_AGCCTRL0				0x1D	// AGC Control
#define CC1101_WOREVT1				0x1E	// High Byte Event0 Timeout
#define CC1101_WOREVT0				0x1F	// Low Byte Event0 Timeout
#define CC1101_WORCTRL				0x20	// Wake On Radio Control
#define CC1101_FREND1				0x21	// Front End RX Configuration
#define CC1101_FREND0				0x22	// Front End TX Configuration
#define CC1101_FSCAL3				0x23	// Frequency Synthesizer Calibration
#define CC1101_FSCAL2				0x24	// Frequency Synthesizer Calibration
#define CC1101_FSCAL1				0x25	// Frequency Synthesizer Calibration
#define CC1101_FSCAL0				0x26	// Frequency Synthesizer Calibration
#define CC1101_RCCTRL1				0x27	// RC Oscillator Configuration
#define CC1101_RCCTRL0				0x28	// RC Oscillator Configuration
#define CC1101_FSTEST				0x29	// Frequency Synthesizer Calibration Control
#define CC1101_PTEST				0x2A	// Production Test
#define CC1101_AGCTEST				0x2B	// AGC Test
#define CC1101_TEST2				0x2C	// Various Test Settings
#define CC1101_TEST1				0x2D	// Various Test Settings
#define CC1101_TEST0				0x2E	// Various Test Settings

/**
 * Status registers
 */
#define CC1101_PARTNUM				0x30	// Chip ID
#define CC1101_VERSION				0x31	// Chip ID
#define CC1101_FREQEST				0x32	// Frequency Offset Estimate from Demodulator
#define CC1101_LQI					0x33	// Demodulator Estimate for Link Quality
#define CC1101_RSSI					0x34	// Received Signal Strength Indication
#define CC1101_MARCSTATE			0x35	// Main Radio Control State Machine State
#define CC1101_WORTIME1				0x36	// High Byte of WOR Time
#define CC1101_WORTIME0				0x37	// Low Byte of WOR Time
#define CC1101_PKTSTATUS			0x38	// Current GDOx Status and Packet Status
#define CC1101_VCO_VC_DAC			0x39	// Current Setting from PLL Calibration Module
#define CC1101_TXBYTES				0x3A	// Underflow and Number of Bytes
#define CC1101_RXBYTES				0x3B	// Overflow and Number of Bytes
#define CC1101_RCCTRL1_STATUS		0x3C	// Last RC Oscillator Calibration Result
#define CC1101_RCCTRL0_STATUS		0x3D	// Last RC Oscillator Calibration Result 

/**
 * Default settings (868,3 MHz @ 38,4 KBaud Hekatron setup)
 */
#define CC1101_DEFVAL_IOCFG2		0x2E	// GDO2 Output Pin Configuration
#define CC1101_DEFVAL_IOCFG1		0x2E	// GDO1 Output Pin Configuration
#define CC1101_DEFVAL_IOCFG0		0x06	// GDO0 Output Pin Configuration
#define CC1101_DEFVAL_FIFOTHR		0x47    // RX FIFO and TX FIFO Thresholds
#define CC1101_DEFVAL_SYNC1			0xD3    // Synchronization word, high byte
#define CC1101_DEFVAL_SYNC0			0x91    // Synchronization word, low byte
#define CC1101_DEFVAL_PKTLEN		0xFF    // Packet Length
#define CC1101_DEFVAL_PKTCTRL1		0x04	// Packet Automation Control
#define CC1101_DEFVAL_PKTCTRL0	 	0x45	// Packet Automation Control
#define CC1101_DEFVAL_ADDR			0x00    // Device Address
#define CC1101_DEFVAL_CHANNR		0x00	// Channel Number
#define CC1101_DEFVAL_FSCTRL1		0x0B	// Frequency Synthesizer Control
#define CC1101_DEFVAL_FSCTRL0		0x00	// Frequency Synthesizer Control
#define CC1101_DEFVAL_FREQ2		  	0x21	// Frequency Control Word, High Byte
#define CC1101_DEFVAL_FREQ1			0x65	// Frequency Control Word, Middle Byte
#define CC1101_DEFVAL_FREQ0			0x6A	// Frequency Control Word, Low Byte
#define CC1101_DEFVAL_MDMCFG4		0x0A	// Modem configuration
#define CC1101_DEFVAL_MDMCFG3		0x83	// Modem Configuration
#define CC1101_DEFVAL_MDMCFG2		0x13    // Modem Configuration
#define CC1101_DEFVAL_MDMCFG1		0x23    // Modem Configuration
#define CC1101_DEFVAL_MDMCFG0		0x99    // Modem Configuration
#define CC1101_DEFVAL_DEVIATN		0x35	// Modem Deviation Setting
#define CC1101_DEFVAL_MCSM2			0x07	// Main Radio Control State Machine Configuration
#define CC1101_DEFVAL_MCSM1			0x3D	// Main Radio Control State Machine Configuration	// CCA: If RSSI below threshold unless currently receiving a packet, RX->RX, TX->FSTXON
#define CC1101_DEFVAL_MCSM0			0x18	// Main Radio Control State Machine Configuration
#define CC1101_DEFVAL_FOCCFG		0x16	// Frequency Offset Compensation Configuration
#define CC1101_DEFVAL_BSCFG			0x1C    // Bit Synchronization Configuration
#define CC1101_DEFVAL_AGCCTRL2	 	0xC7    // AGC Control
#define CC1101_DEFVAL_AGCCTRL1	 	0x00    // AGC Control
#define CC1101_DEFVAL_AGCCTRL0	 	0xB2    // AGC Control
#define CC1101_DEFVAL_WOREVT1		0x87	// High Byte Event0 Timeout
#define CC1101_DEFVAL_WOREVT0		0x6B	// Low Byte Event0 Timeout
#define CC1101_DEFVAL_WORCTRL		0xFB	// Wake On Radio Control
#define CC1101_DEFVAL_FREND1		0x56	// Front End RX Configuration
#define CC1101_DEFVAL_FREND0		0x10	// Front End TX Configuration
#define CC1101_DEFVAL_FSCAL3		0xE9	// Frequency Synthesizer Calibration
#define CC1101_DEFVAL_FSCAL2		0x2A	// Frequency Synthesizer Calibration
#define CC1101_DEFVAL_FSCAL1		0x00	// Frequency Synthesizer Calibration
#define CC1101_DEFVAL_FSCAL0		0x1F	// Frequency Synthesizer Calibration
#define CC1101_DEFVAL_RCCTRL1		0x41	// RC Oscillator Configuration
#define CC1101_DEFVAL_RCCTRL0		0x00	// RC Oscillator Configuration
#define CC1101_DEFVAL_FSTEST		0x59	// Frequency Synthesizer Calibration Control
#define CC1101_DEFVAL_PTEST			0x7F	// Production Test
#define CC1101_DEFVAL_AGCTEST		0x3F	// AGC Test
#define CC1101_DEFVAL_TEST2			0x81	// Various Test Settings
#define CC1101_DEFVAL_TEST1			0x35	// Various Test Settings
#define CC1101_DEFVAL_TEST0			0x09	// Various Test Settings

/**
 * CC1101 Bits
 */
#define RXFIFO_OVERFLOW				0x80	// RX FIFO overflow

/**
 * CC1101 MARC STATES
 */
#define CC1101_SLEEP				0x00
#define CC1101_IDLE					0x01
#define CC1101_XOFF					0x02
#define CC1101_MANCAL_VCOON_MC		0x03
#define CC1101_MANCAL_REGON_MC		0x04
#define CC1101_MANCAL				0x05
#define CC1101_FS_WAKEUP_VCOON		0x06
#define CC1101_FS_WAKEUP_REGON		0x07
#define CC1101_CALIBRATE_STARTCAL	0x08
#define CC1101_SETTLING_BWBOOST		0x09
#define CC1101_SETTLING_FS_LOCK		0x0A
#define CC1101_SETTLING_IFADCON		0x0B
#define CC1101_CALIBRATE_ENDCAL		0x0C
#define CC1101_RX					0x0D
#define CC1101_RX_END				0x0E
#define CC1101_RX_RST				0x0F
#define CC1101_TXRX_SETTLING_SWITCH	0x10
#define CC1101_RXFIFO_OVERFLOW		0x11
#define CC1101_FSTXON				0x12
#define CC1101_TX					0x13
#define CC1101_TX_END				0x14
#define CC1101_RXTX_SETTLING_SWITCH	0x15
#define TXFIFO_UNDERFLOW			0x16


/**
 * Buffers and data lengths
 */
#define CC1101_FIFO_SIZE					64
#define NUM_LENGTH_BYTES					1
#define NUM_STATUS_BYTES					2
#define NUM_ADDITIONAL_BYTES				(NUM_LENGTH_BYTES + NUM_STATUS_BYTES)
#define CC1101_MAX_PACKET_LEN				(CC1101_FIFO_SIZE - NUM_ADDITIONAL_BYTES)

typedef enum cc1101_mode {
	CCM_IDLE = 0,
	CCM_RX,
	CCM_TX
} cc1101_mode_t;

/**
 * CC1101 data packet class
 */
typedef struct cc1101_packet {
	/* Timestamp of packet */
	uint64_t timestamp;
	/* Data buffer containing the original CC1101 RX fifo content
	 * including length byte and status bytes (lqi, rssi, CRC) */
	unsigned char buffer[CC1101_FIFO_SIZE];
	/* Pointer to the actual packet data */
	unsigned char* data;
	/* Length of the packet data */
	size_t length;
} cc1101_packet_t;

/**
 * @brief Initialize CC1101 radio controller
 * 
 * Initialize CC1101 radio and received packet notification (via ISR and task notification)
 *
 * @param gpio_isr_handler ISR handler to be executed on a fully received packet
 */
esp_err_t cc1101_init(void (*rx_callback)());


/**
 * @brief Set CC1101 to RX mode
 * 
 * Strobes the RX command strobe SRX to CC1101. This will set the CC1101 to RX mode.
 * 
 * @return ESP_OK if successful, ESP_FAIL otherwise
 */
esp_err_t cc1101_set_rx_state(void);

/**
 * @brief Set CC1101 to TX mode
 * 
 * Strobes the TX command strobe STX to CC1101. This will set the CC1101 to TX mode.
 * 
 * @return ESP_OK if successful, ESP_FAIL otherwise
 */
esp_err_t cc1101_set_tx_state(void);
	
/**
 * @brief Get data received by CC1101.
 * @details Reads the data packet from RX FIFO.
 * 
 * @return ESP_OK if successful, ESP_FAIL otherwise
 */
esp_err_t cc1101_receive_data(cc1101_packet_t *packet);

/**
 * @brief Send data via CC1101.
 * @details Copies the data to TX FIFO.
 * 
 * @param tx_data Data to be sent
 * @param length Length of the data to be sent
 * 
 * @return ESP_OK if successful, ESP_ERR_INVALID_ARG if tx_data is NULL, ESP_ERR_INVALID_SIZE if length is invalid, ESP_FAIL otherwise
 */
esp_err_t cc1101_send_data(unsigned char *tx_data, size_t length);

/**
 * @brief Check if RX FIFO has overflowed. If so, flush RX FIFO and set CC1101 to RX state.
 * 
 * @return ESP_OK if RX FIFO is empty, ESP_FAIL otherwise
 */
esp_err_t cc1101_check_rx(void);


/**
 * @brief Get the current mode of CC1101.
 * 
 * @return Current mode of CC1101
 */
cc1101_mode_t cc1101_get_mode(void);

/**
 * @brief Get the current control state machine state (MARCSTATE).
 * @param[out] state Pointer to store the current state
 * 
 * @return ESP_OK if successful, otherwise an esp_err_t error code
 */
esp_err_t cc1101_get_state(uint8_t *state);

#ifdef __cplusplus
}
#endif
