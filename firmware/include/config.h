#ifndef FILAMENT_DRYER_CONFIG_H
#define FILAMENT_DRYER_CONFIG_H

#include <avrkit/TWI.h>


/*
  Size for the SD1306-driven display
 */

#define SSD1306_DISPLAY_SIZE__128x32 0x00
#define SSD1306_DISPLAY_SIZE__128x64 0x01

#define SSD1306_DISPLAY_SIZE SSD1306_DISPLAY_SIZE__128x64


// Max running time allowed in hours
#define MAX_RUNNING_TIME_HOURS 96

// Clock frequency for I2C protocol
//#define TWI_FREQ TWI_FREQ_STANDARD
#define TWI_FREQ TWI_FREQ_FAST

// I2C identifier for the SSD1306-driven display
#define SSD1306_I2C_ADDRESS 0x3c

// I2C identifier for the SHT21-DIS temperature and humidity sensor
#define SHT31_I2C_ADDRESS 0x44


#endif /* FILAMENT_DRYER_CONFIG_H */
