#include <util/twi.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <avrkit/GPIO.h>

#include "config.h"


extern const __flash uint8_t bitmap_data[512];


// --- TWI handling -----------------------------------------------------------

#define F_SCL 100000UL // Clock frequency for I2C protocol
//#define F_SCL 400000UL // Clock frequency for I2C protocol


void
twi_init() {
	// Set pin 0 for write operation, set high
	DDRC |= _BV(DDC4);
	PORTC |= _BV(PORTC4);

	// Set pin 1 for write operation, set high
	DDRC |= _BV(DDC5);
	PORTC |= _BV(PORTC5);

	// TWI registers setup
	TWBR = (uint8_t)(((F_CPU / F_SCL) - 16) / 2);
}


void
twi_start() {
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);
}


void
twi_stop() {
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}


void
twi_send_slave_address(uint8_t address) {
	TWDR = (address << 1) | TW_WRITE;
	TWCR = (1 << TWINT) | (1 << TWEN);
	loop_until_bit_is_set(TWCR, TWINT);
}


void
twi_send_data(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	loop_until_bit_is_set(TWCR, TWINT);
}


// --- SSD1306 handling -------------------------------------------------------

#define START_PAGE_ADDR           0
#define END_PAGE_ADDR             3
#define START_COLUMN_ADDR         0
#define END_COLUMN_ADDR           127
  
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

static const uint8_t SSD1306_slave_address = SSD1306_I2C_ADDRESS;

/*
 *  init sequence for 128x32 display
 */

static const  __flash uint8_t 
SSD1306_init_sequence[] = {
	// number of commands
	19,
	0, SSD1306_DISPLAY_OFF,          // Set display off
	1, SSD1306_SET_MUX_RATIO, 0x1f,  // Set mutiplex ratio to 1/64	
	1, SSD1306_MEMORY_ADDR_MODE, 0x00, 
	0, SSD1306_SET_START_LINE,       // Set display start line
	1, SSD1306_DISPLAY_OFFSET, 0x00, // Set display offset to 0
	0, SSD1306_SEG_REMAP_OP,         // Set segment Re-Map default
	0, SSD1306_COM_SCAN_DIR_OP,      // Set COM Output Scan Direction
	0, SSD1306_COM_PIN_CONF, 0x02,   // Set COM hardware configuration
	1, SSD1306_SET_CONTRAST, 0x50,   // Set contrast
	0, SSD1306_DIS_ENT_DISP_ON,      // Entire display ON
	0, SSD1306_DIS_NORMAL,           // Set normal display	
	1, SSD1306_SET_OSC_FREQ, 0x80,   // Set display clock to 105 hz
	1, SSD1306_SET_PRECHARGE, 0xc2,  // Set Pre-Charge period 
	1, SSD1306_VCOM_DESELECT, 0x20,  // Set Deselect Vcomh level	
	1, SSD1306_SET_CHAR_REG, 0x14,   // Set charge pump
	0, SSD1306_DEACT_SCROLL,         // Deactivate scrolling
	2, SSD1306_SET_COLUMN_ADDR, START_COLUMN_ADDR, END_COLUMN_ADDR,
	2, SSD1306_SET_PAGE_ADDR, START_PAGE_ADDR, END_PAGE_ADDR,
	0, SSD1306_DISPLAY_ON,           // Display ON
};


uint8_t
ssd1306_init() {
    // Send START
    twi_start();
    if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
        return 0;
    
    // Send slave address
    twi_send_slave_address(SSD1306_slave_address);
    if (TW_STATUS != TW_MT_SLA_ACK)
        return 0;
    
    // Send SSD1306 startup sequence
    const __flash uint8_t* command_array_ptr = SSD1306_init_sequence;
    uint8_t command_count = *command_array_ptr++;
    for( ; command_count != 0; --command_count) {
        uint8_t arg_count = *command_array_ptr++;
        twi_send_data(SSD1306_COMMAND);
        twi_send_data(*command_array_ptr++);
        
        for( ;  arg_count != 0; --arg_count) {
            twi_send_data(SSD1306_COMMAND);
            twi_send_data(*command_array_ptr++);        
        }
    }
    
    // Send stop
    twi_stop();
    
    // Job done;
    return 1;
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
        twi_send_data(0x0);   
        if (TW_STATUS != TW_MT_DATA_ACK)
            return 0;
    }
    
    // Send stop
    twi_stop();
    
    // Job done;
    return 1;
}


uint8_t
ssd1306_upload_bitmap(const __flash uint8_t* bitmap) {
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


// --- Main function ----------------------------------------------------------

int
main(void) {
	// Setup
	twi_init();
	sei();

	uint8_t ret = ssd1306_init();
	ssd1306_upload_bitmap(bitmap_data);
	//ssd1306_clear();
	//ssd1306_setup_horizontal_scroll(0, 3, 1);
	//ssd1306_activate_scroll();
	
	// Main loop
	while(1)
		sleep_mode();
}
