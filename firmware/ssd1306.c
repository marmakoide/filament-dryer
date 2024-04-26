#include "config.h"
#include "twi.h"
#include "ssd1306.h"


#define SSD1306_COMMAND           0x80  // Continuation bit=1, D/C=0; 1000 0000
#define SSD1306_COMMAND_STREAM    0x00  // Continuation bit=0, D/C=0; 0000 0000
#define SSD1306_DATA              0xc0  // Continuation bit=1, D/C=1; 1100 0000
#define SSD1306_DATA_STREAM       0x40  // Continuation bit=0, D/C=1; 0100 0000

#define SSD1306_SET_MUX_RATIO     0xa8
#define SSD1306_DISPLAY_OFFSET    0xd3
#define SSD1306_DISPLAY_ON        0xaf
#define SSD1306_DISPLAY_OFF       0xae
#define SSD1306_DIS_ENT_DISP_ON   0xa4
#define SSD1306_DIS_IGNORE_RAM    0xa5
#define SSD1306_DIS_NORMAL        0xa6
#define SSD1306_DIS_INVERSE       0xa7
#define SSD1306_DEACT_SCROLL      0x2e
#define SSD1306_ACTIVE_SCROLL     0x2f
#define SSD1306_SET_START_LINE    0x40
#define SSD1306_MEMORY_ADDR_MODE  0x20
#define SSD1306_SET_COLUMN_ADDR   0x21
#define SSD1306_SET_PAGE_ADDR     0x22
#define SSD1306_SEG_REMAP         0xa0
#define SSD1306_SEG_REMAP_OP      0xa1
#define SSD1306_COM_SCAN_DIR      0xc0
#define SSD1306_COM_SCAN_DIR_OP   0xc8
#define SSD1306_COM_PIN_CONF      0xda
#define SSD1306_SET_CONTRAST      0x81
#define SSD1306_SET_OSC_FREQ      0xd5
#define SSD1306_SET_CHAR_REG      0x8d
#define SSD1306_SET_PRECHARGE     0xd9
#define SSD1306_VCOM_DESELECT     0xdb

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2a  ///< Init diag scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xa3             ///< Set scroll range


/*
 * I2C address for the display
 */
 
static const uint8_t 
SSD1306_slave_address = SSD1306_I2C_ADDRESS;


/*
 *  init sequence for 128x32 display
 */

#define START_PAGE_ADDR           0
#define END_PAGE_ADDR             3
#define START_COLUMN_ADDR         0
#define END_COLUMN_ADDR           127


static const  __flash uint8_t 
SSD1306_128x32_init_sequence[] = {
	SSD1306_DISPLAY_OFF,          // Set display off
	SSD1306_SET_MUX_RATIO, 0x1f,  // Set mutiplex ratio to 1/64
	SSD1306_MEMORY_ADDR_MODE, 0x00, 
	SSD1306_SET_START_LINE,       // Set display start line
	SSD1306_DISPLAY_OFFSET, 0x00, // Set display offset to 0
	SSD1306_SEG_REMAP_OP,         // Set segment Re-Map default
	SSD1306_COM_SCAN_DIR_OP,      // Set COM Output Scan Direction
	SSD1306_COM_PIN_CONF, 0x02,   // Set COM hardware configuration
	SSD1306_SET_CONTRAST, 0x50,   // Set contrast
	SSD1306_DIS_ENT_DISP_ON,      // Entire display ON
	SSD1306_DIS_NORMAL,           // Set normal display
	SSD1306_SET_OSC_FREQ, 0x80,   // Set display clock to 105 hz
	SSD1306_SET_PRECHARGE, 0xc2,  // Set Pre-Charge period 
	SSD1306_VCOM_DESELECT, 0x20,  // Set Deselect Vcomh level
	SSD1306_SET_CHAR_REG, 0x14,   // Set charge pump
	SSD1306_DEACT_SCROLL,         // Deactivate scrolling
	SSD1306_DISPLAY_ON,           // Display ON
};


/*
 *  init sequence for 128x64 display
 */

/*
#define START_PAGE_ADDR           0
#define END_PAGE_ADDR             7
#define START_COLUMN_ADDR         0
#define END_COLUMN_ADDR           127


static const  __flash uint8_t 
SSD1306_128x64_init_sequence[] = {
	// number of commands
	19,
	0, SSD1306_DISPLAY_OFF,          // Set display off
	1, SSD1306_SET_OSC_FREQ, 0x80,   // Set display clock to 105 hz	
	1, SSD1306_SET_MUX_RATIO, 0x3f,  // Set mutiplex ratio to 1/64
	1, SSD1306_DISPLAY_OFFSET, 31, // Set display offset to 0
	0, SSD1306_SET_START_LINE | 0x00,       // Set display start line
	1, SSD1306_SET_CHAR_REG, 0x14,   // Set charge pump
	1, SSD1306_MEMORY_ADDR_MODE, 0x00, 
	0, SSD1306_SEG_REMAP_OP,         // Set segment Re-Map default
	0, SSD1306_COM_SCAN_DIR_OP,      // Set COM Output Scan Direction
	0, SSD1306_COM_PIN_CONF, 0x12,   // Set COM hardware configuration
	1, SSD1306_SET_CONTRAST, 0xcf,   // Set contrast
	1, SSD1306_SET_PRECHARGE, 0xf1,  // Set Pre-Charge period 
	1, SSD1306_VCOM_DESELECT, 0x40,  // Set Deselect Vcomh level
	0, SSD1306_DIS_ENT_DISP_ON,      // Entire display ON	
	0, SSD1306_DIS_NORMAL,           // Set normal display	
	0, SSD1306_DEACT_SCROLL,         // Deactivate scrolling
	2, SSD1306_SET_COLUMN_ADDR, START_COLUMN_ADDR, END_COLUMN_ADDR,
	2, SSD1306_SET_PAGE_ADDR, START_PAGE_ADDR, END_PAGE_ADDR,	
	0, SSD1306_DISPLAY_ON,           // Display ON
};
*/


static uint8_t
ssd1306_send_command_stream(const __flash uint8_t* data, uint16_t data_size) {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send SSD1306 startup sequence
	twi_send_data(SSD1306_COMMAND_STREAM);
	if (TW_STATUS != TW_MT_DATA_ACK)
		return 0;

	const __flash uint8_t* command_array_ptr = data;
	for( ; data_size != 0; --data_size) {
		twi_send_data(*command_array_ptr++);
		if (TW_STATUS != TW_MT_DATA_ACK)
			return 0;
	}

	// Send stop
	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_init() {
    return
    	ssd1306_send_command_stream(
    		SSD1306_128x32_init_sequence,
    		sizeof(SSD1306_128x32_init_sequence)
    	);
}


uint8_t
ssd1306_clear() {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send the bitmap data as a stream
	twi_send_data(SSD1306_DATA_STREAM);
	if (TW_STATUS != TW_MT_DATA_ACK)
		return 0;

	for(uint16_t i = 512; i != 0; --i) {
		twi_send_data(0x00);
		if (TW_STATUS != TW_MT_DATA_ACK)
			return 0;
	}

	// Send stop
	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_upload_framebuffer(const __flash uint8_t* bitmap) {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send the bitmap data as a stream
	twi_send_data(SSD1306_DATA_STREAM);
	if (TW_STATUS != TW_MT_DATA_ACK)
		return 0;

	const __flash uint8_t* pixel = bitmap;
	for(uint16_t i = 512; i != 0; --i, ++pixel) {
		twi_send_data(*pixel);
		if (TW_STATUS != TW_MT_DATA_ACK)
			return 0;
	}

	// Send stop
	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_set_display_on() {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send command
	twi_send_data(SSD1306_COMMAND);
	twi_send_data(SSD1306_DISPLAY_ON);
	    
	// Send stop
	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_set_display_off() {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send command
	twi_send_data(SSD1306_COMMAND);
	twi_send_data(SSD1306_DISPLAY_OFF);

	// Send stop
	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_set_normal_display_mode() {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send command
	twi_send_data(SSD1306_COMMAND);
	twi_send_data(SSD1306_DIS_NORMAL);

	// Send stop
	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_set_inverse_display_mode() {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send command
	twi_send_data(SSD1306_COMMAND);
	twi_send_data(SSD1306_DIS_INVERSE);

	// Send stop
	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_activate_scroll() {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send command
	twi_send_data(SSD1306_COMMAND);
	twi_send_data(SSD1306_ACTIVE_SCROLL);

	// Send stop
	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_deactivate_scroll() {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send command
	twi_send_data(SSD1306_COMMAND);
	twi_send_data(SSD1306_DEACT_SCROLL);

	// Send stop
	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_setup_horizontal_scroll(uint8_t start, uint8_t stop, int left_to_right) {
	twi_start();
	twi_send_slave_address(SSD1306_slave_address);

	twi_send_data(SSD1306_COMMAND_STREAM);

	if (left_to_right)
		twi_send_data(SSD1306_RIGHT_HORIZONTAL_SCROLL);
	else
		twi_send_data(SSD1306_LEFT_HORIZONTAL_SCROLL);

	twi_send_data(0x00);
	twi_send_data(start);
	twi_send_data(0x00);
	twi_send_data(stop);
	twi_send_data(0x00);
	twi_send_data(0xff);

	twi_stop();

	// Job done;
	return 1;
}


uint8_t
ssd1306_set_vertical_offset(int8_t offset) {
	// Send START
	twi_start();
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	// Send slave address
	twi_send_slave_address(SSD1306_slave_address);
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	// Send command
	twi_send_data(SSD1306_COMMAND);
	twi_send_data(SSD1306_DISPLAY_OFFSET);
	twi_send_data(offset);

	// Send stop
	twi_stop();

	// Job done;
	return 1;
}
