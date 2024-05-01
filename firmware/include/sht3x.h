#ifndef FILAMENT_DRYER_SHT3X_DIS_H
#define FILAMENT_DRYER_SHT3X_DIS_H

#include <stdint.h>


enum sht3x_measure_repeatability {
	sht3x_measure_repeatability_low    = 0,
	sht3x_measure_repeatability_medium = 1,
	sht3x_measure_repeatability_high   = 2,
}; // enum sht3x_measure_repeatability


extern uint8_t
sht3x_soft_reset();


extern uint8_t
sht3x_enable_header();


extern uint8_t
sht3x_disable_header();


extern uint8_t
sht3x_request_single_shot_measure(
	enum sht3x_measure_repeatability repeatability
);


extern uint8_t
sht3x_acquire_measure(
	int16_t* temperature,
	uint16_t* humidity
);


#endif /* FILAMENT_DRYER_SHT3X_DIS_H */
