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


enum RotaryEncoderEvent {
	RotaryEncoderEvent_left  = 0,
	RotaryEncoderEvent_none  = 1,
	RotaryEncoderEvent_right = 2
};

volatile uint8_t push_button_state = 0;
volatile enum RotaryEncoderEvent rotary_encoder_event = RotaryEncoderEvent_none;

volatile uint16_t tick_counter = 0;
volatile uint8_t target_temperature = 50;
volatile uint8_t remaining_hours = 1;
volatile uint8_t remaining_minutes = 0;


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


// --- Rotary encoder switch handling -----------------------------------------

#define INSTANCIATE_PUSH_BUTTON_SETUP(PIN) \
static void \
setup_push_button() { \
	gpio_pin_##PIN##__set_as_input(); \
	gpio_pin_##PIN##__set_high(); \
	gpio_pin_##PIN##__enable_change_interrupt(); \
} \
\
\
static void \
update_push_button_state() { \
	push_button_state = !gpio_pin_##PIN##__is_high(); \
}


INSTANCIATE_PUSH_BUTTON_SETUP(B0)


// pin 7 => clock
// pin 6 => dt
static void
setup_rotary_encoder() {
	gpio_pin_D7__set_as_input();
	gpio_pin_D7__set_high();
	gpio_pin_D7__enable_change_interrupt();

	gpio_pin_D6__set_as_input();
	gpio_pin_D6__set_high();
}


static void
update_rotary_encoder_event() {
	uint8_t a = gpio_pin_D7__is_high();
	uint8_t b = gpio_pin_D6__is_high();

	if (a) {
		if (b)
			rotary_encoder_event = RotaryEncoderEvent_left;
		else
			rotary_encoder_event = RotaryEncoderEvent_right;
	}
	else
		rotary_encoder_event = RotaryEncoderEvent_none;
}


// --- Interrupt handlers -----------------------------------------------------

ISR(PCINT2_vect) {
	update_rotary_encoder_event();
}

ISR(PCINT0_vect) {
	update_push_button_state();

}

ISR(TIMER1_COMPA_vect) {
	tick_counter += 1;
}


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
	setup_rotary_encoder();
	
	// Allow interrupts
	sei();

	// Display setup
	ssd1306_init();
}


// --- Main entry point -------------------------------------------------------

static void
increase_remaining_time() {
	if (remaining_hours >= 48)
		return;

	remaining_minutes += 1;
	if (remaining_minutes == 60) {
		remaining_hours += 1;
		remaining_minutes = 0;
	}
}


static void
decrease_remaining_time() {
	if ((remaining_hours == 0) && (remaining_minutes == 0))
		return;

	if (remaining_minutes > 0)
		remaining_minutes -= 1;
	else if (remaining_hours > 0) {
		remaining_minutes = 59;
		remaining_hours -= 1;
	}
}


static void
render_charmap() {
	charmap_clear();
	
	if (push_button_state)
		charmap_print(3, 2, "on");
	else
		charmap_print(3, 2, "off");

	charmap_print(2, 0, "%02u:%02u", remaining_hours, remaining_minutes);
	charmap_print(3, 1, "%uc", target_temperature);	
}


int
main(void) {
	setup();
	
	// Main loop
	while(1) {
		/*
		// Update state
		while(tick_counter >= 50) { // 3000 ticks at 50 Hz => 1 minute
			tick_counter -= 50;
			decrease_remaining_time();			
		}
		*/
		
		switch(rotary_encoder_event) {
			case RotaryEncoderEvent_left:
				decrease_remaining_time();
				break;
			
			case RotaryEncoderEvent_right:
				increase_remaining_time();
				break;
				
			default:
				break;
		}
		rotary_encoder_event = RotaryEncoderEvent_none;
		
		// Refresh the display
		render_charmap();
		ssd1306_upload_charmap_16x16(font16x16_data, charmap);	

		// Goes to sleep, awaken every 1/50 secs
		sleep_mode();
	}
}
