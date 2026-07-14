################################################################################
# @file    build.mk
# @brief   Shared compiler, linker, and artifact rules for AKOS examples.
#
# @author  BUUPQ
# @date    Created: 2026-07-13
# @date    Updated: 2026-07-13
#
# @module  AKOS
################################################################################

# ============================================================
# 1. Toolchain
# ============================================================

TOOLCHAIN ?= arm-none-eabi-
CC := $(TOOLCHAIN)gcc
OBJCOPY := $(TOOLCHAIN)objcopy
SIZE := $(TOOLCHAIN)size

# ============================================================
# 2. Build Output
# ============================================================

BUILD_DIR := $(ROOT_DIR)/build/$(BOARD)/$(EXAMPLE)
BOARD_NAME := $(shell printf '%s' '$(BOARD)' | tr '[:upper:]' '[:lower:]')
TARGET_NAME := akos_$(BOARD_NAME)_$(EXAMPLE)
TARGET := $(BUILD_DIR)/$(TARGET_NAME)

# ============================================================
# 3. CPU Selection
# ============================================================

ifeq ($(ARCH),cortex-m0)
CPU_FLAGS := -mcpu=cortex-m0 -mthumb
else ifeq ($(ARCH),cortex-m3)
CPU_FLAGS := -mcpu=cortex-m3 -mthumb
endif

# ============================================================
# 4. Build Flags
# ============================================================

CFLAGS += $(CPU_FLAGS) -std=c11 -Os -g3 -ffunction-sections -fdata-sections \
	-Wall -Wextra -MMD -MP
ASFLAGS += $(CPU_FLAGS) -g3
LDFLAGS += $(CPU_FLAGS) $(KERNEL_LDFLAGS) -T$(LINKER_SCRIPT) \
	-Wl,--gc-sections -Wl,-Map=$(TARGET).map --specs=nano.specs \
	--specs=nosys.specs
LDLIBS += -lc -lm -lnosys

# ============================================================
# 5. Derived Build Inputs
# ============================================================

C_OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.c=.o)))
OBJECTS := $(C_OBJECTS:.s=.o)
DEPS := $(OBJECTS:.o=.d)

# ============================================================
# 6. Default Target
# ============================================================

.DEFAULT_GOAL := all

# ============================================================
# 7. Build Targets
# ============================================================

.PHONY: all clean help

all: $(TARGET).elf $(TARGET).hex $(TARGET).bin
	$(call AKOS_SIZE,$(TARGET).elf)

$(TARGET).elf: $(OBJECTS) $(LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	$(call AKOS_LOG,LD,,$(call AKOS_DISPLAY,$@))
	$(AKOS_Q)$(CC) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(TARGET).hex: $(TARGET).elf
	$(call AKOS_LOG,HEX,,$(call AKOS_DISPLAY,$@))
	$(AKOS_Q)$(OBJCOPY) -O ihex $< $@

$(TARGET).bin: $(TARGET).elf
	$(call AKOS_LOG,BIN,,$(call AKOS_DISPLAY,$@))
	$(AKOS_Q)$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(call AKOS_LOG,CC,,$(call AKOS_DISPLAY,$<))
	$(AKOS_Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(call AKOS_LOG,AS,,$(call AKOS_DISPLAY,$<))
	$(AKOS_Q)$(CC) $(ASFLAGS) -c $< -o $@

clean:
	$(call AKOS_LOG_PATH,CLEAN,,$(BUILD_DIR))
	$(AKOS_Q)rm -rf $(BUILD_DIR)

help:
	@echo "make [BOARD=STM32F030F4P6|STM32F103C8T6|STM32L151CBT6] [clean] [all]"

# ============================================================
# 8. Dependencies
# ============================================================

-include $(DEPS)
