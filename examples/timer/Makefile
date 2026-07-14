################################################################################
# @file    Makefile
# @brief   Build configuration for the AKOS timer example.
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
EXAMPLE := timer
BOARD ?= STM32F030F4P6

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
VPATH += $(EXAMPLE_SRC_DIR)
SOURCES += \
	$(EXAMPLE_SRC_DIR)/main.c \
	$(EXAMPLE_SRC_DIR)/task_timer.c
include ../build.mk
