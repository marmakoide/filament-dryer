include config.mk

MCU=atmega328p
F_CPU=16000000UL
TARGET=firmware.hex

SRC=./src
INCLUDES=./include
AVRKIT_INCLUDES=$(AVRKIT_PATH)/include

OBJ_LIST=\
$(SRC)/firmware.o \
$(SRC)/stringstream.o \
$(SRC)/sht3x.o \
$(SRC)/ssd1306.o \
$(SRC)/twi.o \
$(SRC)/font-8x8.o \
$(SRC)/font-16x16.o


.PHONY: clean upload

all: $(TARGET)

$(SRC)/font-8x8.c: ./assets/font-8x8.png ./bitmap-tool
	python3 ./bitmap-tool to-font --font-format=8x8 --array-name=font8x8_data $< > $@

$(SRC)/font-16x16.c: ./assets/font-16x16.png ./bitmap-tool
	python3 ./bitmap-tool to-font --font-format=16x16 --array-name=font16x16_data $< > $@

$(SRC)/font-8x8.o: $(SRC)/font-8x8.c

$(SRC)/font-16x16.o: $(SRC)/font-16x16.c

$(SRC)/twi.o: $(SRC)/twi.c $(INCLUDES)/twi.h

$(SRC)/sht3x.o: $(SRC)/sht3x.c $(INCLUDES)/sht3x.h $(INCLUDES)/twi.h

$(SRC)/ssd1306.o: $(SRC)/ssd1306.c $(INCLUDES)/ssd1306.h $(INCLUDES)/twi.h

$(SRC)/stringstream.o: $(SRC)/stringstream.c $(INCLUDES)/stringstream.h

$(SRC)/firmware.o: $(SRC)/firmware.c $(INCLUDES)/config.h  $(INCLUDES)/stringstream.h $(INCLUDES)/ssd1306.h $(INCLUDES)/sht3x.h

.c.o:
	avr-gcc -I$(AVRKIT_INCLUDES) -I$(INCLUDES) -Os -DF_CPU=$(F_CPU) -D__AVRKIT_$(MCU)__ -mmcu=$(MCU) -c -o $@ $<

firmware.elf: $(OBJ_LIST)
	avr-gcc -mmcu=$(MCU) $^ -o $@

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom $< $@

clean:
	rm -f $(SRC)/*.o *.elf $(SRC)/font-8x8.c $(SRC)/font-16x16.c $(TARGET)

upload: $(TARGET)
	avrdude -F -V -c arduino -p $(MCU) -P $(USB_PORT) -b 115200 -U flash:w:$<
