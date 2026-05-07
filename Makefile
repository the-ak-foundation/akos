ROOT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

ARCH ?= cortex-m3

COMMON_INCLUDES := \
	-I$(ROOT_DIR) \
	-I$(ROOT_DIR)/kernel \
	-I$(ROOT_DIR)/kernel/inc \
	-I$(ROOT_DIR)/port

COMMON_VPATH := \
	$(ROOT_DIR)/kernel \
	$(ROOT_DIR)/kernel/src

COMMON_SOURCES := \
	$(wildcard $(ROOT_DIR)/kernel/*.c) \
	$(wildcard $(ROOT_DIR)/kernel/src/*.c)

ifeq ($(ARCH),cortex-m3)
ARCH_DIR := $(ROOT_DIR)/port/arm/cortex-m3
ARCH_INCLUDES := -I$(ARCH_DIR)
ARCH_VPATH := $(ARCH_DIR)
ARCH_SOURCES := $(wildcard $(ARCH_DIR)/*.c)
else
$(error Unsupported ARCH '$(ARCH)'. Supported values: cortex-m3)
endif

CFLAGS		+= $(COMMON_INCLUDES) $(ARCH_INCLUDES)
CPPFLAGS	+= $(COMMON_INCLUDES) $(ARCH_INCLUDES)
VPATH += $(COMMON_VPATH) $(ARCH_VPATH)
SOURCES += $(COMMON_SOURCES) $(ARCH_SOURCES)
