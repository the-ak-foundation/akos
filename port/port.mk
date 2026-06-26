################################################################################
# @file    port.mk
# @brief   AKOS port selection and architecture-specific build inputs.
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

PORT_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
ROOT_DIR ?= $(abspath $(PORT_DIR)/..)

ARCH ?= cortex-m3

# ============================================================
# 2. Architecture
# ============================================================

ifeq ($(ARCH),cortex-m3)
    ARCH_DIR := $(PORT_DIR)/ARM/cortex-m3
    ARCH_INCLUDES := -I$(ARCH_DIR)
    ARCH_VPATH := $(ARCH_DIR)
    ARCH_SOURCES := $(wildcard $(ARCH_DIR)/*.c)
else ifeq ($(ARCH),cortex-m0)
    ARCH_DIR := $(PORT_DIR)/ARM/cortex-m0
    ARCH_INCLUDES := -I$(ARCH_DIR)
    ARCH_VPATH := $(ARCH_DIR)
    ARCH_SOURCES := $(wildcard $(ARCH_DIR)/*.c)
else
    $(error Unsupported ARCH '$(ARCH)'. Supported values: cortex-m0, cortex-m3)
endif
