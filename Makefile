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

BOARD ?= AK_BASE_KIT_STM32L151
EXAMPLE ?= thread
TOOLCHAIN ?= arm-none-eabi-

EXAMPLE_DIR := examples/$(EXAMPLE)
EXAMPLE_MAKEFILE := $(EXAMPLE_DIR)/Makefile

# ============================================================
# 2. Validation
# ============================================================

ifeq ($(wildcard $(EXAMPLE_MAKEFILE)),)
$(error Unknown EXAMPLE '$(EXAMPLE)')
endif

# ============================================================
# 3. Default Target
# ============================================================

.DEFAULT_GOAL := all

# ============================================================
# 4. Build Targets
# ============================================================

.PHONY: all clean help

all clean:
	@$(MAKE) -C $(EXAMPLE_DIR) BOARD=$(BOARD) TOOLCHAIN=$(TOOLCHAIN) $@

help:
	@echo "make [BOARD=<board>] [EXAMPLE=<example>] [TOOLCHAIN=<prefix>]"
	@echo ""
	@echo "Boards:   AK_BASE_KIT_STM32L151, STM32F030F4P6, STM32F103C8T6, STM32L151CBT6"
	@echo "Examples: thread, message, priority, timer"
	@echo "Toolchain prefix (default): $(TOOLCHAIN)"
	@echo ""
	@echo "Examples:"
	@echo "  make BOARD=AK_BASE_KIT_STM32L151 EXAMPLE=thread"
	@echo "  make BOARD=STM32F103C8T6 EXAMPLE=message"
