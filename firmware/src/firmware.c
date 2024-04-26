#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <avrkit/GPIO.h>

#include "twi.h"
#include "ssd1306.h"
#include "config.h"


extern const __flash uint8_t
font_data[];


extern const __flash uint8_t
framebuffer_data[];


static char charmap[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV0123456789ABCDEFGHIJKLMNOPQRSTUV0123456789ABCDEFGHIJKLMNOPQRSTUV0123456789ABCDEFGHIJKLMNOPQRSTUV";


int
main(void) {
	// Setup
	twi_init();
	sei();

	uint8_t ret = ssd1306_init();
	//ssd1306_upload_framebuffer(framebuffer_data);
	ssd1306_upload_charmap(font_data, charmap);
	//ssd1306_setup_horizontal_scroll(0, 2, 1, ssd1306_scroll_speed__2);
	//ssd1306_activate_scroll();
	
	// Main loop
	while(1)
		sleep_mode();
}
