# ============================================================
# 1. Platform
# ============================================================

PLATFORM_DIR := $(BOARD_DIR)/platform
PLATFORM_DEVICE_DIR := $(PLATFORM_DIR)/device/TI/MSPM0C110x

# ============================================================
# 2. Includes
# ============================================================

PLATFORM_INCLUDES := \
	-I$(PLATFORM_DIR) \
	-I$(PLATFORM_DEVICE_DIR) \
	-I$(PLATFORM_DEVICE_DIR)/CMSIS/Core/Include

# ============================================================
# 3. Sources
# ============================================================

PLATFORM_SOURCES := \
	$(PLATFORM_DIR)/system.c \
	$(PLATFORM_DIR)/startup.c
