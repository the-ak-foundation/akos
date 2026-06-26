# ============================================================
# 1. Platform
# ============================================================

PLATFORM_DIR := $(BOARD_DIR)/platform
PLATFORM_DEVICE_DIR := $(PLATFORM_DIR)/device/ST/STM32F0xx

# ============================================================
# 2. Includes
# ============================================================

PLATFORM_INCLUDES := \
	-I$(PLATFORM_DIR) \
	-I$(PLATFORM_DEVICE_DIR) \
	-I$(PLATFORM_DEVICE_DIR)/drivers \
	-I$(PLATFORM_DEVICE_DIR)/drivers/Legacy

# ============================================================
# 3. Sources
# ============================================================

PLATFORM_SOURCES := \
	$(PLATFORM_DIR)/system.c \
	$(PLATFORM_DIR)/startup.c
