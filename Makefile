################################################################################
# @file    Makefile
# @brief   Root build dispatcher for AKOS examples.
#
# @author  BUUPQ
# @date    Created: 2026-07-13
# @date    Updated: 2026-07-13
#
# @module  AKOS
################################################################################

# ============================================================
# 1. Build Selection
# ============================================================

BOARD ?= AK_BASE_KIT_V2_3
EXAMPLE ?= thread
TOOLCHAIN ?= arm-none-eabi-

EXAMPLE_DIR := examples/$(EXAMPLE)
PROJECT_DIR := projects/$(EXAMPLE)

ifneq ($(wildcard $(EXAMPLE_DIR)/Makefile),)
BUILD_DIR := $(EXAMPLE_DIR)
else ifneq ($(wildcard $(PROJECT_DIR)/Makefile),)
BUILD_DIR := $(PROJECT_DIR)
else
BUILD_DIR :=
endif

BUILD_MAKEFILE := $(BUILD_DIR)/Makefile

# ============================================================
# 2. Validation
# ============================================================

ifeq ($(BUILD_DIR),)
$(error Unknown EXAMPLE '$(EXAMPLE)')
endif

# ============================================================
# 3. Default Target
# ============================================================

.DEFAULT_GOAL := all

# ============================================================
# 4. Build Targets
# ============================================================

.PHONY: all clean flash size test help

all clean flash size test:
	@$(MAKE) -C $(BUILD_DIR) BOARD=$(BOARD) TOOLCHAIN=$(TOOLCHAIN) $@

help:
	@echo "make [BOARD=<board>] [EXAMPLE=<example>] [TOOLCHAIN=<prefix>]"
	@echo ""
	@echo "Boards:   AK_BASE_KIT_V2_3, STM32F030F4P6, STM32F103C8T6, STM32L151CBT6"
	@echo "Examples: thread, message, priority, timer"
	@echo "Projects: game_hub"
	@echo "Toolchain prefix (default): $(TOOLCHAIN)"
	@echo ""
	@echo "Examples:"
	@echo "  make BOARD=AK_BASE_KIT_V2_3 EXAMPLE=thread"
	@echo "  make BOARD=STM32F103C8T6 EXAMPLE=message"
	@echo "  make EXAMPLE=game_hub"
	@echo "  make EXAMPLE=game_hub flash FLASH_PORT=/dev/ttyUSB0"
