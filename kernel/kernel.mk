################################################################################
# @file    kernel.mk
# @brief   Common AKOS kernel build inputs.
#
# @author  HUYNNG | BUUPQ
# @date    Created: 2026-06-11
# @date    Updated: 2026-06-26
#
# @module  AKOS
################################################################################

# ============================================================
# 1. Project
# ============================================================

KERNEL_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
ROOT_DIR ?= $(abspath $(KERNEL_DIR)/..)
PORT_DIR ?= $(ROOT_DIR)/port

# ============================================================
# 2. Config
# ============================================================

include $(KERNEL_DIR)/config.mk
include $(PORT_DIR)/port.mk

# ============================================================
# 3. Common Paths
# ============================================================

KERNEL_INCLUDES := \
	-I$(ROOT_DIR) \
	-I$(KERNEL_DIR) \
	-I$(KERNEL_DIR)/inc \
	-I$(ROOT_DIR)/port

KERNEL_VPATH := \
	$(KERNEL_DIR) \
	$(KERNEL_DIR)/src

KERNEL_SOURCES := \
	$(wildcard $(KERNEL_DIR)/*.c) \
	$(wildcard $(KERNEL_DIR)/src/*.c)

KERNEL_LDFLAGS := \
	-Wl,-L$(KERNEL_DIR)

# ============================================================
# 4. Exported Build Inputs
# ============================================================

CPPFLAGS += $(KERNEL_INCLUDES) $(ARCH_INCLUDES) $(KERNEL_CONFIG_DEFINES)
VPATH += $(KERNEL_VPATH) $(ARCH_VPATH)
SOURCES += $(KERNEL_SOURCES) $(ARCH_SOURCES)
