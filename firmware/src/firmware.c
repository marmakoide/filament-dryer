#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <avrkit/GPIO.h>

#include "twi.h"
#include "ssd1306.h"
#include "config.h"


// --- Flash data ------------------------------------------------------------

extern const __flash uint8_t
font8x8_data[];


extern const __flash uint8_t
font16x16_data[];


// --- RAM data ---------------------------------------------------------------

static char charmap[32];
const static uint8_t charmap_width = 8;

static uint8_t push_button_state = 0;


// --- Charmap handling -------------------------------------------------------

static void
charmap_clear() {
	memset(charmap, 0, sizeof(charmap));
}


static void
charmap_print(
	uint8_t x,
	uint8_t y,
	const char* format,
	...) {
	char* charmap_ptr = charmap;
	charmap_ptr += charmap_width * y + x;

	va_list argp;
	va_start(argp, format);
	vsnprintf(charmap_ptr, charmap_width - x, format, argp);
}


// --- Push button handling ---------------------------------------------------

#define INSTANCIATE_PUSH_BUTTON_SETUP(PIN) \
static void \
setup_push_button() { \
	gpio_pin_##PIN##__set_as_input(); \
	gpio_pin_##PIN##__set_high(); \
	gpio_pin_##PIN##__enable_change_interrupt(); \
} \
\
static void \
update_push_button_state() { \
	push_button_state = !gpio_pin_##PIN##__is_high(); \
}

INSTANCIATE_PUSH_BUTTON_SETUP(B0)


// --- Interrupt handlers -----------------------------------------------------

ISR(PCINT0_vect) {
	update_push_button_state();

}

ISR(TIMER1_COMPA_vect) { }


// --- Setup code -------------------------------------------------------------

static void
setup_timer1() {
	// Clear timer on compare match
	TCCR1B |= _BV(WGM12);

	// Set prescaler to 256
	TCCR1B |= _BV(CS12);

	// Reset timer after 1250 ticks ie. 1/50 sec on a (16/256) clock
	OCR1A = 1249;
	
	// Trigger TIMER1_COMPA interruption
	TIMSK1 |= _BV(OCIE1A);
}


static void
setup() {
	twi_init();
	setup_timer1();
	setup_push_button();

	// Allow interrupts
	sei();

	// Display setup
	ssd1306_init();
}


// --- Main entry point -------------------------------------------------------


int
main(void) {
	setup();
	
	// Main loop
	while(1) {
		// Refresh the display
		charmap_clear();
		
		if (push_button_state)
			charmap_print(3, 2, "on");
		else
			charmap_print(3, 2, "off");
		
		ssd1306_upload_charmap_16x16(font16x16_data, charmap);	

		// Goes to sleep, awaken every 1/50 secs
		sleep_mode();
	}
}
