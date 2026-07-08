PROJECT      := akos-00-blink

SAMPLE_DIR   := $(abspath .)
BUILD_DIR    := $(SAMPLE_DIR)/build
TARGET       := $(BUILD_DIR)/$(PROJECT).elf
BIN          := $(BUILD_DIR)/$(PROJECT).bin
MAP          := $(BUILD_DIR)/$(PROJECT).map

include $(abspath ../..)/Makefile

ARCH         ?= cortex-m3
CROSS_PREFIX ?= arm-none-eabi-
CC           := $(CROSS_PREFIX)gcc
OBJCOPY      := $(CROSS_PREFIX)objcopy
SIZE         := $(CROSS_PREFIX)size
AK_FLASH     ?= ak-flash
ST_FLASH     ?= st-flash
ST_FLASH_FLAGS ?= --connect-under-reset --reset
FLASH_ADDR   ?= 0x08003000

MCU          := cortex-m3
CPUFLAGS     := -mcpu=$(MCU) -mthumb
DEFINES      := -DSTM32L1XX_MD -DSTM32L151xB -DAKOS_CFG_USE_LOG=0 -DAKOS_CFG_USE_SHELL=0 -DAKOS_CFG_USE_CLI=0
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

OBJECTS := $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(basename $(patsubst $(ROOT_DIR)/%,%,$(SOURCES)))))

.PHONY: all clean size bin flash

all: $(TARGET) $(BIN)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BIN): $(TARGET)
	@mkdir -p $(@D)
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(COMMON_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.s
	@mkdir -p $(@D)
	$(CC) $(COMMON_ASFLAGS) -c $< -o $@

size: $(TARGET)
	$(SIZE) $<

bin: $(BIN)

flash: $(BIN)
ifneq ($(strip $(dev)),)
	$(AK_FLASH) $(dev) $(BIN) $(FLASH_ADDR)
else
	$(ST_FLASH) $(ST_FLASH_FLAGS) write $(BIN) $(FLASH_ADDR)
endif

clean:
	rm -rf $(BUILD_DIR)
