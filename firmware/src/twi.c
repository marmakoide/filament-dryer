#include "twi.h"
#include "config.h"


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
