#ifndef FILAMENT_DRYER_CONFIG_H
#define FILAMENT_DRYER_CONFIG_H

/*
  Size for the SD1306-driven display
 */

#define SSD1306_DISPLAY_SIZE__128x32 0x00
#define SSD1306_DISPLAY_SIZE__128x64 0x01

#define SSD1306_DISPLAY_SIZE SSD1306_DISPLAY_SIZE__128x64


// Clock frequency for I2C protocol
//#define F_SCL 100000UL
#define F_SCL 400000UL 

// I2C identifier for the SSD1306-driven display
#define SSD1306_I2C_ADDRESS 0x3c

// I2C identifier for the SHT21-DIS temperature and humidity sensor
#define SHT31_DIS_I2C_ADDRESS 0x44


#endif /* FILAMENT_DRYER_CONFIG_H */
