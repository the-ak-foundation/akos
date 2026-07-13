################################################################################
# @file    board.mk
# @brief   Build configuration for the STM32F030F4P6 board.
#
# @author  BUUPQ
# @date    Created: 2026-07-13
# @date    Updated: 2026-07-13
#
# @module  AKOS
################################################################################

# ============================================================
# 1. Project
# ============================================================

BOARD_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
ROOT_DIR ?= $(abspath $(BOARD_DIR)/../..)

# ============================================================
# 2. Board Configuration
# ============================================================

MCU := STM32F030F4P6
OS_CFG_SYSTICK_CLOCK_HZ ?= 48000000u
OS_CFG_USE_LOG ?= 0u

# ============================================================
# 3. Kernel
# ============================================================

include $(ROOT_DIR)/kernel/kernel.mk

# ============================================================
# 4. Exported Build Inputs
# ============================================================

CPPFLAGS += -I$(BOARD_DIR)
VPATH += $(BOARD_DIR)
SOURCES += $(BOARD_DIR)/board.c
LINKER_SCRIPT := $(BOARD_DIR)/linker.ld
