PROJECT      := akos-00-blink
TARGET       := $(PROJECT).elf
BIN          := $(PROJECT).bin
MAP          := $(PROJECT).map

SAMPLE_DIR   := $(abspath .)

include $(abspath ../..)/Makefile

ARCH         ?= cortex-m3
CROSS_PREFIX ?= arm-none-eabi-
CC           := $(CROSS_PREFIX)gcc
OBJCOPY      := $(CROSS_PREFIX)objcopy
SIZE         := $(CROSS_PREFIX)size
ST_FLASH     ?= st-flash
ST_FLASH_FLAGS ?= --connect-under-reset --reset
FLASH_ADDR   ?= 0x08000000

MCU          := cortex-m3
CPUFLAGS     := -mcpu=$(MCU) -mthumb
DEFINES      := -DSTM32L1XX_MD -DSTM32L151xB -DOS_CFG_USE_LOG=0 -DOS_CFG_USE_CLI=0
COMMON_CFLAGS += \
	$(CFLAGS) \
	$(CPUFLAGS) \
	$(DEFINES) \
	-I$(SAMPLE_DIR) \
	-include $(SAMPLE_DIR)/stm32l1xx.h \
	-ffreestanding \
	-fdata-sections \
	-fno-common \
	-fno-builtin \
	-Os \
	-Wall \
	-Wextra

COMMON_ASFLAGS += \
	$(CPPFLAGS) \
	$(CPUFLAGS) \
	$(DEFINES) \
	-I$(SAMPLE_DIR) \
	-x assembler-with-cpp

LDFLAGS := \
	$(CPUFLAGS) \
	-T$(SAMPLE_DIR)/stm32l151xx.ld \
	-static \
	-nostartfiles \
	-Wl,-Map=$(MAP) \
	-Wl,--gc-sections \
	-Wl,--cref

SOURCES := \
	$(SOURCES) \
	$(SAMPLE_DIR)/system_stm32l1xx.c \
	$(SAMPLE_DIR)/main.c \
	$(SAMPLE_DIR)/startup_stm32l151xb.s

OBJECTS := $(SOURCES:.c=.o)
OBJECTS := $(OBJECTS:.s=.o)

.PHONY: all clean size bin flash

all: $(TARGET) $(BIN)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BIN): $(TARGET)
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(COMMON_CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(COMMON_ASFLAGS) -c $< -o $@

size: $(TARGET)
	$(SIZE) $<

bin: $(BIN)

flash: $(BIN)
	$(ST_FLASH) $(ST_FLASH_FLAGS) write $(BIN) $(FLASH_ADDR)

clean:
	rm -f $(OBJECTS) $(TARGET) $(BIN) $(MAP)
