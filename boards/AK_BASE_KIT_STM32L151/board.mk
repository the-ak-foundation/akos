################################################################################
# @file    board.mk
# @brief   Application build configuration for the AK Base Kit STM32L151.
#
# @author  BUUPQ
# @date    Created: 2026-07-15
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

MCU := STM32L151CBT6
ARCH := cortex-m3
OS_CFG_SYSTICK_CLOCK_HZ ?= 32000000u
OS_CFG_USE_LOG ?= 0u

THIRD_PARTY_DIR := $(BOARD_DIR)/third_party
CMSIS_DIR := $(THIRD_PARTY_DIR)/cmsis
DEVICE_DIR := $(THIRD_PARTY_DIR)/device

# The AK Base Kit bootloader jumps to the application vector table at 0x08003000.
APP_FLASH_ADDRESS := 0x08003000

define BOARD_POST_BUILD
@printf '\n\033[1;33mFLASH COMMAND:\033[0m\n'
@printf 'Run from: %s\n' '$(BUILD_DIR)'
@printf 'ak-flash /dev/ttyUSB0 %s %s\n\n' '$(notdir $(TARGET).bin)' '$(APP_FLASH_ADDRESS)'
endef

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
	-DSTM32L151xB \
	-DHSE_VALUE=8000000u \
	-DHSI_VALUE=16000000u \
	-DHSE_STARTUP_TIMEOUT=0x5000u \
	-DAKOS_PORT_DEVICE_HEADER=\"stm32l1xx.h\"
VPATH += $(BOARD_DIR) $(DEVICE_DIR)/src
SOURCES += \
	$(BOARD_DIR)/bsp.c \
	$(DEVICE_DIR)/src/startup_stm32l151xb.s \
	$(DEVICE_DIR)/src/system_stm32l1xx.c

LINKER_SCRIPT := $(BOARD_DIR)/linker.ld
