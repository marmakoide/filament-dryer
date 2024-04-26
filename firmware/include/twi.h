#ifndef FILAMENT_DRYER_TWI_H
#define FILAMENT_DRYER_TWI_H

#include <util/twi.h>
#include <stdint.h>


extern void
twi_init();

extern void
twi_start();

extern void
twi_stop();

extern void
twi_send_slave_address(uint8_t address);

extern void
twi_send_data(uint8_t data);


#endif /* FILAMENT_DRYER_TWI_H */
