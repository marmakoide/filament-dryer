AVRKIT_PATH=/home/alex/Labo/embedded/arduino/avrkit
USB_PORT=/dev/ttyUSB0

MCU=atmega328p
F_CPU=16000000UL

CC=avr-gcc 
CFLAGS=-Os -DF_CPU=$(F_CPU) -D__AVRKIT_$(MCU)__ -mmcu=$(MCU)
