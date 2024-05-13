#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <avrkit/GPIO.h>
#include <avrkit/TWI.h>
#include <avrkit/drivers/sht3x.h>

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
static char display_status_line[16];
static char display_target_temp_line[8];
static char display_remaining_time_line[8];


// Clock ticks (at 50Hz, would wrap around after about 21 minutes)
volatile uint16_t tick_counter = 0;


// Push button
enum PushButtonEvent {
	PushButtonEvent__none = 0,
	PushButtonEvent__maybe_pressed = 1,
	PushButtonEvent__pressed = 2,
	PushButtonEvent__maybe_released = 3,
	PushButtonEvent__released = 4
}; // enum PushButtonEvent

volatile uint8_t push_button_press = 0;
enum PushButtonEvent push_button_event = PushButtonEvent__none;


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

enum UserInterfaceStatus user_interface_status = UserInterfaceStatus__default;


// Remaining time
uint8_t remaining_hours = 0; // Remanining drying time
uint8_t remaining_minutes = 0;


// Target temperature
uint8_t target_temperature = 50; // Temperature to maintain


// Temperature and humidity measures
struct sht3x_measure measure = { 0, 0 };
uint8_t measure_acquired = 0; // Flag to check if we acquired a measure


// --- Display rendering ------------------------------------------------------

static void
render_display_status_line(struct StringStream* stream) {
	// Bound the measured temperature and humidity to fit in the display
	int16_t display_temperature = measure.temperature;
	uint16_t display_humidity = measure.humidity;
	
	if (display_temperature < 0)
		display_temperature = 0;

	if (display_temperature > 990)
		display_temperature = 990;

	if (display_humidity > 990)
		display_humidity = 990;	

	// Render the measured temperature and humidity
	StringStream_enable_inverse_mode(stream);
	StringStream_push_char(stream, ' ');	
	StringStream_push_uint8(stream, display_temperature / 10, 2);
	StringStream_push_char(stream, 'c');

	StringStream_push_nchar(stream, 8, ' ');
	
	StringStream_push_uint8(stream, display_humidity / 10, 2);
	StringStream_push_char(stream, '%');
	StringStream_push_char(stream, ' ');
	StringStream_disable_inverse_mode(stream);	
}


static void
render_display_target_temp_line(struct StringStream* stream) {
	if (user_interface_status == UserInterfaceStatus__target_temperature_selected)
		StringStream_enable_inverse_mode(stream);
		
	StringStream_push_nchar(stream, 3, ' ');
	StringStream_push_uint8(stream, target_temperature, 2);
	StringStream_push_char(stream, 'c');
	StringStream_push_nchar(stream, 2, ' ');
	
	if (user_interface_status == UserInterfaceStatus__target_temperature_selected)
		StringStream_disable_inverse_mode(stream);
}


static void
render_display_remaining_time_line(struct StringStream* stream) {
	if (user_interface_status == UserInterfaceStatus__remaining_time_selected)
		StringStream_enable_inverse_mode(stream);
	
	StringStream_push_nchar(stream, 2, ' ');
	StringStream_push_uint8(stream, remaining_hours, 2);
	StringStream_push_char(stream, ':');
	StringStream_push_uint8(stream, remaining_minutes, 2);
	StringStream_push_char(stream, ' ');
	
	if (user_interface_status == UserInterfaceStatus__remaining_time_selected)
		StringStream_disable_inverse_mode(stream);
}


static void
render_display() {
	struct StringStream stream;

	// Render the display status line
	if (measure_acquired) {
		StringStream_init(&stream, display_status_line);
		render_display_status_line(&stream);
	}
	
	// Render the target temperature line
	StringStream_init(&stream, display_target_temp_line);
	render_display_target_temp_line(&stream);
	
	// Render the remaining time line
	StringStream_init(&stream, display_remaining_time_line);
	render_display_remaining_time_line(&stream);
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
static uint8_t \
update_push_button_pressed() { \
	push_button_press = !gpio_pin_##PIN##__is_high(); \
}

INSTANCIATE_PUSH_BUTTON_SETUP(B0)


// pin 6 => dt
// pin 7 => clock
static void
setup_rotary_encoder() {
	gpio_pin_D6__set_as_input();
	gpio_pin_D6__set_high();
	gpio_pin_D6__enable_change_interrupt();

	gpio_pin_D7__set_as_input();
	gpio_pin_D7__set_high();
}


static void
update_rotary_encoder_event() {
	uint8_t a = gpio_pin_D6__is_high();
	uint8_t b = gpio_pin_D7__is_high();

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
	update_push_button_pressed();

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
	twi_pins_setup();
	twi_set_speed(TWI_FREQ);
	setup_timer1();
	setup_push_button();
	setup_rotary_encoder();
	
	// Allow interrupts
	sei();

	// SHT3x setup
	sht3x_i2c_address = SHT3X_I2C_ADDRESS;
	
	// Display setup
	ssd1306_init();
}


// --- Main entry point -------------------------------------------------------

static void
increase_target_temperature() {
	if (target_temperature < 99)
		target_temperature += 1;
}


static void
decrease_target_temperature() {
	if (target_temperature > 0)
		target_temperature -= 1;
}


static void
increase_remaining_hours() {
	if (remaining_hours < MAX_RUNNING_TIME_HOURS)
		remaining_hours += 1;
}


static void
decrease_remaining_hours() {
	if (remaining_hours > 0)
		remaining_hours -= 1;
}


static void
increase_remaining_minutes() {
	if (remaining_hours >= MAX_RUNNING_TIME_HOURS)
		return;

	remaining_minutes += 1;
	if (remaining_minutes == 60) {
		remaining_hours += 1;
		remaining_minutes = 0;
	}
}


static void
decrease_remaining_minutes() {
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
		// Update remaining time
		if (tick_counter % 50 == 0) {
			if (user_interface_status == UserInterfaceStatus__default)
				decrease_remaining_minutes();
		}
		
		// Request a temperature and humidity every 50 ticks
		if (tick_counter % 50 == 0) {
			sht3x_acquire_measure(&measure);
			measure_acquired = 1;
			sht3x_request_single_shot_measure(sht3x_measure_repeatability_high);
		}
		
		// Handle button press 
		uint8_t press = push_button_press;
		switch(push_button_event) {
			case PushButtonEvent__none:
				if (press)
					push_button_event = PushButtonEvent__maybe_pressed;
				break;
			
			case PushButtonEvent__maybe_pressed:
				if (press)
					push_button_event = PushButtonEvent__pressed;
				else
					push_button_event = PushButtonEvent__none;
				break;
			
			case PushButtonEvent__pressed:
				if (!press)
					push_button_event = PushButtonEvent__maybe_released;
				break;

			case PushButtonEvent__maybe_released:
				if (!press)
					push_button_event = PushButtonEvent__released;
				else
					push_button_event = PushButtonEvent__pressed;
				break;
			
			case PushButtonEvent__released:
				if (!press)
					push_button_event = PushButtonEvent__none;
				else
					push_button_event = PushButtonEvent__maybe_pressed;
				break;
		}
		
		if (push_button_event == PushButtonEvent__released) {
			switch(user_interface_status) {
				case UserInterfaceStatus__default:
					user_interface_status = UserInterfaceStatus__target_temperature_selected;
					break;

				case UserInterfaceStatus__target_temperature_selected:
					user_interface_status = UserInterfaceStatus__remaining_time_selected;
					break;

				case UserInterfaceStatus__remaining_time_selected:
					user_interface_status = UserInterfaceStatus__default;
					break;
			}
			
			push_button_event == PushButtonEvent__none;
		}

		// Handle rotary encoder event
		switch(rotary_encoder_event) {
			case RotaryEncoderEvent__left:
				switch(user_interface_status) {
					case UserInterfaceStatus__target_temperature_selected:
						decrease_target_temperature();
						break;
				
					case UserInterfaceStatus__remaining_time_selected:
						decrease_remaining_hours();
						break;

					default:
						break;
				}
				break;
			
			case RotaryEncoderEvent__right:
				switch(user_interface_status) {
					case UserInterfaceStatus__target_temperature_selected:
						increase_target_temperature();
						break;
				
					case UserInterfaceStatus__remaining_time_selected:
						increase_remaining_hours();
						break;

					default:
						break;
				}
				break;
				
			default:
				break;
		}
		rotary_encoder_event = RotaryEncoderEvent__none;
		
		// Refresh the display
		render_display();

		ssd1306_upload_start();
		ssd1306_upload_charmap_8x8(font8x8_data, display_status_line, 1);
		ssd1306_upload_empty_row(1);
		ssd1306_upload_charmap_16x16(font16x16_data, display_target_temp_line, 1);
		ssd1306_upload_empty_row(1);
		ssd1306_upload_charmap_16x16(font16x16_data, display_remaining_time_line, 1);
		ssd1306_upload_empty_row(1);
		ssd1306_upload_end();

		// Goes to sleep, awaken every 1/50 secs
		sleep_mode();
	}
}
