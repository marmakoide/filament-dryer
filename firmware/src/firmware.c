#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <string.h>

#include <avrkit/GPIO.h>

#include "twi.h"
#include "sht3x.h"
#include "ssd1306.h"
#include "config.h"
#include "stringstream.h"


// --- Flash data ------------------------------------------------------------

extern const __flash uint8_t
font8x8_data[];


extern const __flash uint8_t
font16x16_data[];


// --- RAM data ---------------------------------------------------------------

// Display data
static char top_charmap[16]; // 1 lines of 16 chars
const static uint8_t top_charmap_width = 16;
const static uint8_t top_charmap_height = 1;

static char bottom_charmap[24]; // 3 lines of 8 chars
const static uint8_t bottom_charmap_width = 8;
const static uint8_t bottom_charmap_height = 3;


// Clock ticks (at 50Hz, would wrap around after about 21 minutes)
volatile uint16_t tick_counter = 0;


// Push button
volatile uint8_t push_button_state = 0;


// Rotary encoder
enum RotaryEncoderEvent {
	RotaryEncoderEvent__left  = 0,
	RotaryEncoderEvent__none  = 1,
	RotaryEncoderEvent__right = 2
}; // enum RotaryEncoderEvent

volatile enum RotaryEncoderEvent rotary_encoder_event = RotaryEncoderEvent__none;


// User interface status
enum UserInterfaceStatus {
	UserInterfaceStatus__default = 0,
	UserInterfaceStatus__target_temperature_selected = 1,
	UserInterfaceStatus__remaining_time_selected = 2
}; // enum UserInterfaceStatus

enum UserInterfaceStatus user_interface_status = UserInterfaceStatus__remaining_time_selected;


// Remaining time
uint8_t remaining_hours = 0; // Remanining drying time
uint8_t remaining_minutes = 0;


// Target temperature
uint8_t target_temperature = 50; // Temperature to maintain


// Temperature and humidity measures
int16_t temperature = 0;      // Current temperature
uint16_t humidity = 0;        // Current humidity
uint8_t measure_acquired = 0; // Flag to check if we acquired a measure


// --- Charmap handling -------------------------------------------------------

static void
charmap_clear() {
	memset(top_charmap, 0, sizeof(top_charmap));
	memset(bottom_charmap, 0, sizeof(bottom_charmap));	
}


static void
charmap_render() {
	struct StringStream stream;

	charmap_clear();
	
	StringStream_init(&stream, top_charmap);
	
	if (user_interface_status == UserInterfaceStatus__target_temperature_selected)
		StringStream_enable_inverse_mode(&stream);
	StringStream_push_char(&stream, ' ');
	StringStream_push_uint8(&stream, target_temperature, 2);
	StringStream_push_char(&stream, 'c');
	StringStream_push_char(&stream, ' ');
	if (user_interface_status == UserInterfaceStatus__target_temperature_selected)
		StringStream_disable_inverse_mode(&stream);

	StringStream_push_nchar(&stream, ' ', 2);

	if (user_interface_status == UserInterfaceStatus__remaining_time_selected)
		StringStream_enable_inverse_mode(&stream);
	StringStream_push_char(&stream, ' ');
	StringStream_push_uint8(&stream, remaining_hours, 2);
	StringStream_push_char(&stream, ':');
	StringStream_push_uint8(&stream, remaining_minutes, 2);
	StringStream_push_char(&stream, ' ');
	if (user_interface_status == UserInterfaceStatus__remaining_time_selected)
		StringStream_disable_inverse_mode(&stream);
	
	if (measure_acquired) {
		// Bound the measured temperature and humidity to fit in the display
		int16_t display_temperature = temperature;
		uint16_t display_humidity = humidity;
		
		if (display_temperature < 0)
			display_temperature = 0;

		if (display_temperature > 990)
			display_temperature = 990;

		if (display_humidity > 990)
			display_humidity = 990;

		// Render the measured temperature and humidity
		StringStream_init(&stream, bottom_charmap);
		StringStream_push_uint8(&stream, display_temperature / 10, 2);
		StringStream_push_char(&stream, 'c');
		StringStream_push_char(&stream, ' ');
		StringStream_push_uint8(&stream, display_humidity / 10, 2);
		StringStream_push_char(&stream, '%');
	}
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
			rotary_encoder_event = RotaryEncoderEvent__left;
		else
			rotary_encoder_event = RotaryEncoderEvent__right;
	}
	else
		rotary_encoder_event = RotaryEncoderEvent__none;
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


int
main(void) {
	setup();

	// Initiate the first measure
	sht3x_request_single_shot_measure(sht3x_measure_repeatability_high);
	
	// Main loop
	while(1) {
		// Request a temperature and humidity every 50 ticks
		if (tick_counter >= 50) {
			tick_counter -= 50;
			sht3x_acquire_measure(&temperature, &humidity);
			measure_acquired = 1;
			sht3x_request_single_shot_measure(sht3x_measure_repeatability_high);
		}

		// UI logic
		switch(rotary_encoder_event) {
			case RotaryEncoderEvent__left:
				decrease_remaining_time();
				break;
			
			case RotaryEncoderEvent__right:
				increase_remaining_time();
				break;
				
			default:
				break;
		}
		rotary_encoder_event = RotaryEncoderEvent__none;
		
		// Refresh the display
		charmap_render();
		ssd1306_upload_start();
		ssd1306_upload_charmap_8x8(font8x8_data, top_charmap, top_charmap_height);
		ssd1306_upload_empty_row(1);
		ssd1306_upload_charmap_16x16(font16x16_data, bottom_charmap, bottom_charmap_height);	
		ssd1306_upload_end();

		// Goes to sleep, awaken every 1/50 secs
		sleep_mode();
	}
}
