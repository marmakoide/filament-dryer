include config.mk

TARGET=firmware.hex

SRC=./src
BUILD=./build
INCLUDES=./include
AVRKIT_INCLUDES=$(AVRKIT_PATH)/include


.PHONY: clean upload

all: $(TARGET)

$(BUILD)/font-8x8.c: ./assets/font-8x8.png ./bitmap-tool
	python3 ./bitmap-tool to-font --font-format=8x8 --array-name=font8x8_data $< > $@

$(BUILD)/font-16x16.c: ./assets/font-16x16.png ./bitmap-tool
	python3 ./bitmap-tool to-font --font-format=16x16 --array-name=font16x16_data $< > $@

$(BUILD)/font-8x8.o: $(BUILD)/font-8x8.c

$(BUILD)/font-16x16.o: $(BUILD)/font-16x16.c

$(BUILD)/twi.o: $(SRC)/twi.c $(INCLUDES)/twi.h

$(BUILD)/sht3x.o: $(SRC)/sht3x.c $(INCLUDES)/sht3x.h $(INCLUDES)/twi.h

$(BUILD)/ssd1306.o: $(SRC)/ssd1306.c $(INCLUDES)/ssd1306.h $(INCLUDES)/twi.h

$(BUILD)/stringstream.o: $(SRC)/stringstream.c $(INCLUDES)/stringstream.h

$(BUILD)/firmware.o: $(SRC)/firmware.c $(INCLUDES)/config.h  $(INCLUDES)/stringstream.h $(INCLUDES)/ssd1306.h $(INCLUDES)/sht3x.h

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir --parents $(BUILD)
	$(CC) -I$(AVRKIT_INCLUDES) -I$(INCLUDES) -o $@ -c $(CFLAGS) $<

$(BUILD)/%.o: $(BUILD)/%.c
	@mkdir --parents $(BUILD)
	$(CC) -o $@ -c $(CFLAGS) $<


firmware.elf: \
$(BUILD)/firmware.o \
$(BUILD)/stringstream.o \
$(BUILD)/sht3x.o \
$(BUILD)/ssd1306.o \
$(BUILD)/twi.o \
$(BUILD)/font-8x8.o \
$(BUILD)/font-16x16.o
	avr-gcc -mmcu=$(MCU) $^ -o $@

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom $< $@

clean:
	@rm -rf $(BUILD)
	@rm -f *.elf $(TARGET)

upload: $(TARGET)
	avrdude -F -V -c arduino -p $(MCU) -P $(USB_PORT) -b 115200 -U flash:w:$<
