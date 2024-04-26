#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <avrkit/GPIO.h>

#include "twi.h"
#include "ssd1306.h"
#include "config.h"


extern const __flash uint8_t
framebuffer_data[512];


int
main(void) {
	// Setup
	twi_init();
	sei();

	uint8_t ret = ssd1306_init();
	ssd1306_upload_framebuffer(framebuffer_data);
	ssd1306_setup_horizontal_scroll(0, 3, 1, ssd1306_scroll_speed__256);
	ssd1306_activate_scroll();
	
	// Main loop
	while(1)
		sleep_mode();
}
