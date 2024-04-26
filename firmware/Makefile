include config.mk

MCU=atmega328p
F_CPU=16000000UL
TARGET=firmware.hex

INCLUDES=$(AVRKIT_PATH)/include


.PHONY: clean upload

all: $(TARGET)

bitmap.c: ./assets/bitmap-small.png ./bitmap-to-framebuffer
	python3 ./bitmap-to-framebuffer $< > $@

bitmap.o: bitmap.c

twi.o: twi.c twi.h

firmware.o: firmware.c config.h

.c.o:
	avr-gcc -I$(INCLUDES) -Os -DF_CPU=$(F_CPU) -D__AVRKIT_$(MCU)__ -mmcu=$(MCU) -c -o $@ $<

firmware.elf: firmware.o twi.o bitmap.o
	avr-gcc -mmcu=$(MCU) $^ -o $@

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom $< $@

clean:
	rm -f *.o *.elf bitmap.c $(TARGET)

upload: $(TARGET)
	avrdude -F -V -c arduino -p $(MCU) -P $(USB_PORT) -b 115200 -U flash:w:$<
