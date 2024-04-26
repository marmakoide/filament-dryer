#ifndef FILAMENT_DRYER_SSD1306_H
#define FILAMENT_DRYER_SSD1306_H

#include <stdint.h>


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
ssd1306_setup_horizontal_scroll(uint8_t start, uint8_t stop, int left_to_right);


extern uint8_t
ssd1306_set_vertical_offset(int8_t offset);


#endif /* FILAMENT_DRYER_SSD1306_H */
