#ifndef FILAMENT_DRYER_SSD1306_H
#define FILAMENT_DRYER_SSD1306_H

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
}; // ssd1306_scroll_speed


extern uint8_t
ssd1306_init();


extern uint8_t
ssd1306_clear();


extern uint8_t
ssd1306_upload_framebuffer(const __flash uint8_t* bitmap);


extern uint8_t
ssd1306_set_display_on();


extern uint8_t
ssd1306_set_display_off();


extern uint8_t
ssd1306_set_normal_display_mode();


extern uint8_t
ssd1306_set_inverse_display_mode();


extern uint8_t
ssd1306_activate_scroll();


extern uint8_t
ssd1306_deactivate_scroll();


extern uint8_t
ssd1306_setup_horizontal_scroll(
	uint8_t start,
	uint8_t stop,
	int left_to_right,
	enum ssd1306_scroll_speed speed
);


extern uint8_t
ssd1306_set_vertical_offset(int8_t offset);


#endif /* FILAMENT_DRYER_SSD1306_H */
