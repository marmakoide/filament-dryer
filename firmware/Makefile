include config.mk

MCU=atmega328p
F_CPU=16000000UL
TARGET=firmware.hex

SRC=./src
INCLUDES=./include
AVRKIT_INCLUDES=$(AVRKIT_PATH)/include


.PHONY: clean upload

all: $(TARGET)

$(SRC)/bitmap.c: ./assets/bitmap.png ./bitmap-tool
	python3 ./bitmap-tool to-framebuffer $< > $@

$(SRC)/font-8x8.c: ./assets/font-8x8.png ./bitmap-tool
	python3 ./bitmap-tool to-font $< > $@

$(SRC)/font-8x8.o: $(SRC)/font-8x8.c

$(SRC)/bitmap.o: $(SRC)/bitmap.c

$(SRC)/twi.o: $(SRC)/twi.c $(INCLUDES)/twi.h

$(SRC)/ssd1306.o: $(SRC)/ssd1306.c $(INCLUDES)/ssd1306.h $(INCLUDES)/twi.h

$(SRC)/firmware.o: $(SRC)/firmware.c $(INCLUDES)/config.h

.c.o:
	avr-gcc -I$(AVRKIT_INCLUDES) -I$(INCLUDES) -Os -DF_CPU=$(F_CPU) -D__AVRKIT_$(MCU)__ -mmcu=$(MCU) -c -o $@ $<

firmware.elf: $(SRC)/firmware.o $(SRC)/ssd1306.o $(SRC)/twi.o $(SRC)/font-8x8.o $(SRC)/bitmap.o
	avr-gcc -mmcu=$(MCU) $^ -o $@

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom $< $@

clean:
	rm -f $(SRC)/*.o *.elf $(SRC)/bitmap.c $(SRC)/font-8x8.c $(TARGET)

upload: $(TARGET)
	avrdude -F -V -c arduino -p $(MCU) -P $(USB_PORT) -b 115200 -U flash:w:$<
