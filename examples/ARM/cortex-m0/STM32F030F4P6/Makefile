# ============================================================
# 1. Project
# ============================================================

ROOT_DIR  := $(shell git rev-parse --show-toplevel)
BOARD_DIR := $(abspath .)

APP ?= 01-blink_led
APP_DIR := $(BOARD_DIR)/$(APP)
APP_MK := $(firstword $(wildcard $(APP_DIR)/*.mk))
APP_LIST := $(notdir $(patsubst %/,%,$(dir $(wildcard $(BOARD_DIR)/*/main.c))))

include $(APP_MK)

ARCH := cortex-m0
OS_CFG_USE_LOG ?= 0u
OS_CFG_USE_CLI ?= 0u

include $(ROOT_DIR)/kernel/kernel.mk

# ============================================================
# 2. Toolchain
# ============================================================

CROSS_PREFIX ?= arm-none-eabi-

CC      := $(CROSS_PREFIX)gcc
OBJCOPY := $(CROSS_PREFIX)objcopy
SIZE    := $(CROSS_PREFIX)size

ST_FLASH ?= st-flash
ST_FLASH_FLAGS ?= --connect-under-reset --reset
FLASH_ADDR ?= 0x08000000

CPUFLAGS := -mcpu=cortex-m0 -mthumb
DEFINES := -DSTM32F030x6
LIB_SPECS := --specs=nosys.specs --specs=nano.specs

# ============================================================
# 3. Sources
# ============================================================

include $(BOARD_DIR)/platform/platform.mk

BUILD_DIR := $(BOARD_DIR)/_build/$(APP)
OBJECT_DIR := $(BUILD_DIR)/objects
IMAGE_DIR := $(BUILD_DIR)/images

APP_TARGET ?= $(APP)
TARGET := $(BUILD_DIR)/$(APP_TARGET).elf
BIN    := $(IMAGE_DIR)/$(APP_TARGET).bin
HEX    := $(IMAGE_DIR)/$(APP_TARGET).hex
MAP    := $(OBJECT_DIR)/$(APP_TARGET).map

ALL_SOURCES := \
	$(SOURCES) \
	$(PLATFORM_SOURCES) \
	$(APP_SOURCES)

OBJECTS := $(addprefix $(OBJECT_DIR)/,$(notdir $(ALL_SOURCES:.c=.o)))

# ============================================================
# 4. Build Flags
# ============================================================

COMMON_CFLAGS += \
	$(CFLAGS) \
	$(CPPFLAGS) \
	$(CPUFLAGS) \
	$(DEFINES) \
	-I$(APP_DIR) \
	$(PLATFORM_INCLUDES) \
	-include stm32f0xx.h \
	-ffreestanding \
	-fdata-sections \
	-fno-common \
	-fno-builtin \
	-Os -g \
	-Wall \
	-Wextra

LDFLAGS := \
	$(CPUFLAGS) \
	$(LIB_SPECS) \
	-T$(BOARD_DIR)/linker/stm32f030f4p6.ld \
	-static \
	-Wl,-Map=$(MAP) \
	-Wl,--gc-sections \
	-Wl,--cref \
	$(KERNEL_LDFLAGS)

VPATH += \
	$(sort $(dir $(ALL_SOURCES)))

# ============================================================
# 5. Targets
# ============================================================

.PHONY: all clean size bin hex flash help

all: $(TARGET) $(BIN) $(HEX)

$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(call AKOS_LOG_PATH,3,Link elf -,$(call AKOS_DISPLAY,$@))
	$(AKOS_Q)$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(call AKOS_LOG_PATH,6,Show size -,$(call AKOS_DISPLAY,$@))
	$(call AKOS_SIZE,$@)

$(BIN): $(TARGET) | $(IMAGE_DIR)
	$(call AKOS_LOG_PATH,4,Make bin -,$(call AKOS_DISPLAY,$@))
	$(AKOS_Q)$(OBJCOPY) -O binary $< $@

$(HEX): $(TARGET) | $(IMAGE_DIR)
	$(call AKOS_LOG_PATH,5,Make hex -,$(call AKOS_DISPLAY,$@))
	$(AKOS_Q)$(OBJCOPY) -O ihex $< $@

$(OBJECT_DIR)/%.o: %.c | $(OBJECT_DIR)
	$(call AKOS_LOG_PATH,2,Build object -,$(call AKOS_DISPLAY,$@))
	$(AKOS_Q)$(CC) $(COMMON_CFLAGS) -c $< -o $@

$(BUILD_DIR) $(OBJECT_DIR) $(IMAGE_DIR):
	$(call AKOS_LOG_PATH,1,Create build dir -,$(call AKOS_DISPLAY,$@))
	$(AKOS_Q)mkdir -p $@

size: $(TARGET)
	$(call AKOS_LOG_PATH,6,Show size -,$(call AKOS_DISPLAY,$<))
	$(call AKOS_SIZE,$<)

bin: $(BIN)

hex: $(HEX)

flash: $(BIN)
	$(ST_FLASH) $(ST_FLASH_FLAGS) write $(BIN) $(FLASH_ADDR)

clean:
	$(call AKOS_LOG_PATH,0,Clean intermediates -,$(call AKOS_DISPLAY,$(BUILD_DIR)))
	$(AKOS_Q)rm -rf $(OBJECT_DIR) $(TARGET) $(MAP)

help:
	@printf "AKOS STM32F030F4P6 example build\n"
	@printf "\n"
	@printf "Usage:\n"
	@printf "  make [target] [APP=<app>] [FLASH_ADDR=<addr>]\n"
	@printf "\n"
	@printf "Apps:\n"
	@for app in $(APP_LIST); do \
		if [ "$$app" = "$(APP)" ]; then \
			printf "  %s (default)\n" "$$app"; \
		else \
			printf "  %s\n" "$$app"; \
		fi; \
	done
	@printf "\n"
	@printf "Targets:\n"
	@printf "  all             Build elf, bin, and hex\n"
	@printf "  clean           Remove intermediates, keep bin and hex\n"
	@printf "  size            Show elf size\n"
	@printf "  bin             Build binary image\n"
	@printf "  hex             Build Intel HEX image\n"
	@printf "  flash           Flash binary with st-flash\n"
	@printf "  help            Show this help\n"
	@printf "\n"
	@printf "Examples:\n"
	@printf "  make clean all\n"
	@printf "  make APP=<app> clean all\n"
	@printf "  make APP=<app> flash\n"
