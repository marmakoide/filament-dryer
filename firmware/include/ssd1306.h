#ifndef FILAMENT_DRYER_SSD1306_H
#define FILAMENT_DRYER_SSD1306_H

/*
 * API to drive a SSD1306 OLED display driver
 *
 * Some assumptions are made to keep things simple
 *   - We use I2C for communications, with a fixed address
 *   - We use blocking communications
 *   - We use horizontal addressing mode
 *
 * No framebuffer is used. Some primitive are provided to send pixels data,
 * it's up to the API user to decide if a framebuffer is needed or not, what
 * size the framebuffer should be, etc. It allows to save a lot of RAM.
 */

#include <stdint.h>


enum ssd1306_scroll_speed {
	ssd1306_scroll_speed__2   = 0x07,
	ssd1306_scroll_speed__3   = 0x04,
	ssd1306_scroll_speed__4   = 0x05,
	ssd1306_scroll_speed__5   = 0x00,
	ssd1306_scroll_speed__25  = 0x06,
	ssd1306_scroll_speed__64  = 0x01,
	ssd1306_scroll_speed__128 = 0x02,
	ssd1306_scroll_speed__256 = 0x03,
}; // enum ssd1306_scroll_speed


/*
 * Initialise the SSD1306 driver, along the assumption we made
 */
extern uint8_t
ssd1306_init();


/*
 * Initialise a pixel data upload
 *
 * NOTE : you are reponsible to send the proper number of pixels
 */
extern uint8_t
ssd1306_upload_start();


/*
 * Terminate a pixel data upload
 *
 * NOTE : you are reponsible to send the proper number of pixels
 */
extern uint8_t
ssd1306_upload_end();


/*
 * Send a full framebuffer stored in flash memory
 */
extern uint8_t
ssd1306_upload_framebuffer(const __flash uint8_t* bitmap);


/*
 * Render characters from a string, using a 8x8 pixels font. Only the lower
 * 7 bits are used for the characters, the upper bit is used to enable 
 * per-character video inverse
 */
extern uint8_t
ssd1306_upload_charmap_8x8(
	const __flash uint8_t* font,
	const char* charmap,
	uint8_t charmap_height
);


/*
 * Render characters from a string, using a 16x16 pixels font. Only the lower
 * 7 bits are used for the characters, the upper bit is used to enable 
 * per-character video inverse
 */
extern uint8_t
ssd1306_upload_charmap_16x16(
	const __flash uint8_t* font,
	const char* charmap,
	uint8_t charmap_height
);


/*
 * Enable the display
 */
extern uint8_t
ssd1306_set_display_on();


/*
 * Disable the display
 */
extern uint8_t
ssd1306_set_display_off();


/*
 * Enable normal display mode, ie. not video inverse
 */
extern uint8_t
ssd1306_set_normal_display_mode();


/*
 * Enable inverse display mode, ie. video inverse
 */
extern uint8_t
ssd1306_set_inverse_display_mode();


/*
 * Setup hardware scrolling
 */
extern uint8_t
ssd1306_setup_horizontal_scroll(
	uint8_t start,
	uint8_t stop,
	int left_to_right,
	enum ssd1306_scroll_speed speed
);


/*
 * Enable hardware scrolling
 *
 * NOTE : you setup the scrolling first
 */
extern uint8_t
ssd1306_activate_scroll();


/*
 * Disable hardware scrolling
 */
extern uint8_t
ssd1306_deactivate_scroll();


/*
 * Offset the scanlines
 *
 * NOTE: On displays smaller thatn 128x64, this allow to use the internal RAM
 * of the SSD1306 as buffer for animations
 */
extern uint8_t
ssd1306_set_vertical_offset(int8_t offset);


#endif /* FILAMENT_DRYER_SSD1306_H */
