PLATFORM_ROOT := $(PROJECT_ROOT)/platform
CMSIS_ROOT := $(PLATFORM_ROOT)/third_party/cmsis
DEVICE_ROOT := $(PLATFORM_ROOT)/third_party/device

CFLAGS += -I./platform
CPPFLAGS += \
	-I./platform \
	-I$(CMSIS_ROOT)/inc \
	-I$(DEVICE_ROOT)/inc \
	-DSTM32L151xB \
	-DHSE_VALUE=8000000u \
	-DHSI_VALUE=16000000u \
	-DHSE_STARTUP_TIMEOUT=0x5000u \
	-DAKOS_PORT_DEVICE_HEADER=\"stm32l1xx.h\"

VPATH += platform $(DEVICE_ROOT)/src

SOURCES += \
	platform/boot_shared.c \
	platform/bsp.c \
	platform/game_hal.c \
	platform/io_cfg.c \
	platform/system.c \
	$(DEVICE_ROOT)/src/startup_stm32l151xb.s \
	$(DEVICE_ROOT)/src/system_stm32l1xx.c

SOURCES_CPP += platform/game_gfx.cpp

LINKER_SCRIPT := $(PLATFORM_ROOT)/linker.ld
