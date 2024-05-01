#ifndef FILAMENT_DRYER_TWI_H
#define FILAMENT_DRYER_TWI_H

#include <stdint.h>


extern void
twi_init();


extern uint8_t
twi_start();


extern void
twi_stop();


extern uint8_t
twi_request_transmission(uint8_t address);


extern uint8_t
twi_request_reception(uint8_t address);


extern uint8_t
twi_transmit(uint8_t data);


extern uint8_t
twi_receive_ack(uint8_t* data);


extern uint8_t
twi_receive_nack(uint8_t* data);


#endif /* FILAMENT_DRYER_TWI_H */
