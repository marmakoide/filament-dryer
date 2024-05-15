#ifndef FILAMENT_DRYER_CONFIG_H
#define FILAMENT_DRYER_CONFIG_H

#include <avrkit/TWI.h>

// Max running time allowed in hours
#define MAX_RUNNING_TIME_HOURS 96

// Clock frequency for I2C protocol
//#define TWI_FREQ TWI_FREQ_STANDARD
#define TWI_FREQ TWI_FREQ_FAST

// I2C identifier for the SSD1306-driven display
#define SSD1306_I2C_ADDRESS 0x3c

// I2C identifier for the SHT31 temperature and humidity sensor
#define SHT3X_I2C_ADDRESS 0x44


#endif /* FILAMENT_DRYER_CONFIG_H */
