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

BOARD ?= STM32F030F4P6
EXAMPLE ?= thread

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
	@$(MAKE) -C $(EXAMPLE_DIR) BOARD=$(BOARD) $@

help:
	@echo "make [BOARD=<board>] [EXAMPLE=<example>]"
	@echo ""
	@echo "Boards:   STM32F030F4P6, STM32F103C8T6, STM32L151CBT6"
	@echo "Examples: thread, message, priority, timer"
