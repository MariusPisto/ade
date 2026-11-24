PROGRAM  = ade
MCU      = atmega328p
F_CPU    = 16000000UL
PORT    ?= COM7
DEMO    ?= light
BUILD    = build/$(DEMO)

DEMOS = $(basename $(notdir $(wildcard src/demo/*.c)))
ifeq ($(filter $(DEMO),$(DEMOS)),)
  $(error Unknown DEMO '$(DEMO)', pick one of: $(DEMOS))
endif

RTOS      = lib/FreeRTOS-Kernel
RTOS_PORT = $(RTOS)/portable/GCC/ATMega328

SRCS = $(wildcard src/*.c src/drivers/*.c $(RTOS)/*.c) $(RTOS_PORT)/port.c \
       src/demo/$(DEMO).c
OBJS = $(SRCS:%.c=$(BUILD)/%.o)

INCLUDES = -Isrc -I$(RTOS)/include -I$(RTOS_PORT)

# Toolchain shipped with the Arduino IDE on Windows, otherwise from PATH
ARDUINO_TOOLS = $(subst \,/,$(LOCALAPPDATA))/Arduino15/packages/arduino/tools
ifneq ($(wildcard $(ARDUINO_TOOLS)/avr-gcc),)
  AVR_BIN     = $(firstword $(wildcard $(ARDUINO_TOOLS)/avr-gcc/*/bin))/
  AVRDUDE_DIR = $(firstword $(wildcard $(ARDUINO_TOOLS)/avrdude/*))
  AVRDUDE     = $(AVRDUDE_DIR)/bin/avrdude -C $(AVRDUDE_DIR)/etc/avrdude.conf
else
  AVRDUDE     = avrdude
endif

CC      = $(AVR_BIN)avr-gcc
OBJCOPY = $(AVR_BIN)avr-objcopy
SIZE    = $(AVR_BIN)avr-size

CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -g -std=gnu11 -Wall -Wextra \
          -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS = -mmcu=$(MCU) -Wl,--gc-sections

all: $(BUILD)/$(PROGRAM).hex
	$(SIZE) --format=avr --mcu=$(MCU) $(BUILD)/$(PROGRAM).elf

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/$(PROGRAM).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD)/$(PROGRAM).hex: $(BUILD)/$(PROGRAM).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

upload: $(BUILD)/$(PROGRAM).hex
	$(AVRDUDE) -c arduino -p $(MCU) -P $(PORT) -b 115200 -D -U flash:w:$<:i

clean:
	rm -rf build

.PHONY: all upload clean

-include $(OBJS:.o=.d)
