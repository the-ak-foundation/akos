PROJECT      := akos-01-blink-cm0
TARGET       := $(PROJECT).elf
BIN          := $(PROJECT).bin
MAP          := $(PROJECT).map

SAMPLE_DIR   := $(abspath .)

ARCH         = cortex-m0
include $(abspath ../..)/Makefile

CROSS_PREFIX ?= arm-none-eabi-
CC           := $(CROSS_PREFIX)gcc
OBJCOPY      := $(CROSS_PREFIX)objcopy
SIZE         := $(CROSS_PREFIX)size
ST_FLASH     ?= st-flash
ST_FLASH_FLAGS ?= --connect-under-reset --reset
FLASH_ADDR   ?= 0x08000000

MCU          := cortex-m0
CPUFLAGS     := -mcpu=$(MCU) -mthumb
DEFINES      := -DSTM32F030x6 -DOS_CFG_USE_LOG=0 -DOS_CFG_USE_CLI=0
COMMON_CFLAGS += \
	$(CFLAGS) \
    $(CPPFLAGS) \
	$(CPUFLAGS) \
	$(DEFINES) \
	-I$(SAMPLE_DIR) \
	-include $(SAMPLE_DIR)/stm32f0xx.h \
	-ffreestanding \
	-fdata-sections \
	-fno-common \
	-fno-builtin \
	-Os -g \
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
    --specs=nosys.specs \
    --specs=nano.specs \
	-T$(SAMPLE_DIR)/stm32f030xx.ld \
	-static \
	-Wl,-Map=$(MAP) \
	-Wl,--gc-sections \
	-Wl,--cref

SOURCES := \
	$(SOURCES) \
	$(SAMPLE_DIR)/system_stm32f0xx.c \
	$(SAMPLE_DIR)/main.c \
	$(SAMPLE_DIR)/startup_stm32f030x6.s

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
