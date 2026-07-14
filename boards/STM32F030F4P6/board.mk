################################################################################
# @file    board.mk
# @brief   Build configuration for the STM32F030F4P6 board.
#
# @author  BUUPQ
# @date    Created: 2026-07-13
# @date    Updated: 2026-07-14
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
ARCH := cortex-m0
OS_CFG_SYSTICK_CLOCK_HZ ?= 48000000u
OS_CFG_USE_LOG ?= 0u

THIRD_PARTY_DIR := $(BOARD_DIR)/third_party
CMSIS_DIR := $(THIRD_PARTY_DIR)/cmsis
DEVICE_DIR := $(THIRD_PARTY_DIR)/device

# ============================================================
# 3. Kernel
# ============================================================

include $(ROOT_DIR)/kernel/kernel.mk

# ============================================================
# 4. Exported Build Inputs
# ============================================================

CPPFLAGS += \
	-I$(BOARD_DIR) \
	-I$(CMSIS_DIR)/inc \
	-I$(DEVICE_DIR)/inc \
	-DSTM32F030x6 \
	-DAKOS_PORT_DEVICE_HEADER=\"stm32f0xx.h\"
VPATH += $(BOARD_DIR) $(DEVICE_DIR)/src
SOURCES += \
	$(BOARD_DIR)/bsp.c \
	$(DEVICE_DIR)/src/system_stm32f0xx.c \
	$(DEVICE_DIR)/src/startup_stm32f030x6.s
LINKER_SCRIPT := $(BOARD_DIR)/linker.ld
