PROJECT      := akos-01-blink-cm0

SAMPLE_DIR   := $(abspath .)
BUILD_DIR    := $(SAMPLE_DIR)/build
TARGET       := $(BUILD_DIR)/$(PROJECT).elf
BIN          := $(BUILD_DIR)/$(PROJECT).bin
MAP          := $(BUILD_DIR)/$(PROJECT).map

ARCH         = cortex-m0
include $(abspath ../..)/Makefile

CROSS_PREFIX ?= arm-none-eabi-
CC           := $(CROSS_PREFIX)gcc
OBJCOPY      := $(CROSS_PREFIX)objcopy
SIZE         := $(CROSS_PREFIX)size
AK_FLASH     ?= ak-flash
ST_FLASH     ?= st-flash
ST_FLASH_FLAGS ?= --connect-under-reset --reset
FLASH_ADDR   ?= 0x08000000

MCU          := cortex-m0
CPUFLAGS     := -mcpu=$(MCU) -mthumb
DEFINES      := -DSTM32F030x6 -DAKOS_CFG_USE_LOG=0 -DAKOS_CFG_USE_SHELL=0 -DAKOS_CFG_USE_CLI=0 \
                -DAKOS_CFG_HEAP_SIZE=1536u -DAKOS_CFG_MSG_POOL_SIZE=8u -DAKOS_CFG_TIMER_POOL_SIZE=2u
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
