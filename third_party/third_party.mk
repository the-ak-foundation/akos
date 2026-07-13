################################################################################
# @file    third_party.mk
# @brief   CMSIS and MCU vendor build inputs for AKOS.
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

THIRD_PARTY_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
ROOT_DIR ?= $(abspath $(THIRD_PARTY_DIR)/..)

CMSIS_DIR := $(THIRD_PARTY_DIR)/cmsis
CMSIS_CORE_DIR := $(CMSIS_DIR)/Core/inc
VENDOR_DIR := $(THIRD_PARTY_DIR)/vendor

MCU ?=

# ============================================================
# 2. Common CMSIS Inputs
# ============================================================

THIRD_PARTY_INCLUDES := -I$(CMSIS_CORE_DIR)
THIRD_PARTY_DEFINES :=
THIRD_PARTY_VPATH :=
THIRD_PARTY_SOURCES :=

# ============================================================
# 3. Device Selection
# ============================================================

ifeq ($(MCU),STM32F030F4P6)
    ARCH ?= cortex-m0
    THIRD_PARTY_DEVICE_DIR := $(VENDOR_DIR)/stm32f0
    THIRD_PARTY_INCLUDES += -I$(THIRD_PARTY_DEVICE_DIR)/inc
    THIRD_PARTY_DEFINES += \
	-DSTM32F030x6 \
	-DAKOS_PORT_DEVICE_HEADER=\"stm32f0xx.h\"
    THIRD_PARTY_VPATH += $(THIRD_PARTY_DEVICE_DIR)/src
    THIRD_PARTY_SOURCES += \
	$(THIRD_PARTY_DEVICE_DIR)/src/system_stm32f0xx.c \
	$(THIRD_PARTY_DEVICE_DIR)/src/startup_stm32f030x6.s
else ifeq ($(MCU),STM32L151CBT6)
    ARCH ?= cortex-m3
    THIRD_PARTY_DEVICE_DIR := $(VENDOR_DIR)/stm32l1
    THIRD_PARTY_INCLUDES += -I$(THIRD_PARTY_DEVICE_DIR)/inc
    THIRD_PARTY_DEFINES += \
	-DSTM32L151xB \
	-DAKOS_PORT_DEVICE_HEADER=\"stm32l1xx.h\"
    THIRD_PARTY_VPATH += $(THIRD_PARTY_DEVICE_DIR)/src
    THIRD_PARTY_SOURCES += \
	$(THIRD_PARTY_DEVICE_DIR)/src/system_stm32l1xx.c \
	$(THIRD_PARTY_DEVICE_DIR)/src/startup_stm32l151xb.s
else ifeq ($(MCU),MSPM0C1104)
    ARCH ?= cortex-m0
    THIRD_PARTY_DEVICE_DIR := $(VENDOR_DIR)/mspm0
    THIRD_PARTY_INCLUDES += -I$(THIRD_PARTY_DEVICE_DIR)/inc
    THIRD_PARTY_DEFINES += \
	-D__MSPM0C1104__ \
	-DAKOS_PORT_DEVICE_HEADER=\"mspm0c110x.h\"
    THIRD_PARTY_VPATH += $(THIRD_PARTY_DEVICE_DIR)/src
    THIRD_PARTY_SOURCES += $(THIRD_PARTY_DEVICE_DIR)/src/startup_mspm0c110x_gcc.c
else ifneq ($(strip $(MCU)),)
    $(error Unsupported MCU '$(MCU)'. Supported values: STM32F030F4P6, STM32L151CBT6, MSPM0C1104)
endif

# ============================================================
# 4. Exported Build Inputs
# ============================================================

CPPFLAGS += $(THIRD_PARTY_INCLUDES) $(THIRD_PARTY_DEFINES)
VPATH += $(THIRD_PARTY_VPATH)
SOURCES += $(THIRD_PARTY_SOURCES)
