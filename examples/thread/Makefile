################################################################################
# @file    Makefile
# @brief   Build configuration for the AKOS thread example.
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

ROOT_DIR := $(abspath ../..)
EXAMPLE := thread
BOARD ?= STM32F030F4P6

# F030 keeps all three tasks but omits per-task formatted logging because its
# 4 KB SRAM cannot safely hold three printf-capable stacks.
ifeq ($(BOARD),STM32F030F4P6)
OS_CFG_HEAP_SIZE ?= 1536u
THREAD_TASK_STACK_SIZE := 32u
THREAD_TASK_LOG_ENABLED := 0u
EXAMPLE_REQUIRED_HEAP_SIZE := 1416u
else
# Logging targets use 256-word stacks and the legacy 8 KB heap size.
OS_CFG_HEAP_SIZE ?= 8192u
THREAD_TASK_STACK_SIZE := 256u
THREAD_TASK_LOG_ENABLED := 1u
EXAMPLE_REQUIRED_HEAP_SIZE := 4104u
endif

BOARD_MK := $(ROOT_DIR)/boards/$(BOARD)/board.mk

# ============================================================
# 2. Board Validation
# ============================================================

ifeq ($(wildcard $(BOARD_MK)),)
$(error Unknown BOARD '$(BOARD)')
endif

# ============================================================
# 3. Build Inputs
# ============================================================

include $(BOARD_MK)
EXAMPLE_INC_DIR := $(CURDIR)/inc
EXAMPLE_SRC_DIR := $(CURDIR)/src
CPPFLAGS += -I$(EXAMPLE_INC_DIR)
CPPFLAGS += \
	-DTHREAD_TASK_STACK_SIZE=$(THREAD_TASK_STACK_SIZE) \
	-DTHREAD_TASK_LOG_ENABLED=$(THREAD_TASK_LOG_ENABLED)
VPATH += $(EXAMPLE_SRC_DIR)

SOURCES += \
	$(EXAMPLE_SRC_DIR)/main.c \
	$(EXAMPLE_SRC_DIR)/task_blink.c
include ../build.mk
