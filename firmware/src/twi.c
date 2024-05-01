#include <util/twi.h>
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


uint8_t
twi_start() {
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);
	
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;
	
	return 1;
}


void
twi_stop() {
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}


uint8_t
twi_request_transmission(uint8_t address) {
	TWDR = (address << 1) | TW_WRITE;
	TWCR = _BV(TWINT) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);
	
	if (TW_STATUS != TW_MT_SLA_ACK)
		return 0;

	return 1;
}


uint8_t
twi_request_reception(uint8_t address) {
	TWDR = (address << 1) | TW_READ;
	TWCR = _BV(TWINT) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);
	
	if (TW_STATUS != TW_MR_SLA_ACK)
		return 0;

	return 1;
}


uint8_t
twi_transmit(uint8_t data) {
	TWDR = data;
	TWCR = _BV(TWINT) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);

	if (TW_STATUS != TW_MT_DATA_ACK)
		return 0;

	return 1;
}


uint8_t
twi_receive_ack(uint8_t* data) {
	TWCR = _BV(TWINT) | _BV(TWEN) | (1 << TWEA);
	loop_until_bit_is_set(TWCR, TWINT);

	if (TW_STATUS != TW_MR_DATA_ACK)
		return 0;
	
	*data = TWDR;

	return 1;
}


uint8_t
twi_receive_nack(uint8_t* data) {
	TWCR = _BV(TWINT) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);

	if (TW_STATUS != TW_MR_DATA_NACK)
		return 0;
	
	*data = TWDR;

	return 1;
}
