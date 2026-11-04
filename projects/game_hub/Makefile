################################################################################
# @file    Makefile
# @brief   Standalone AK Base Kit game project.
################################################################################

ROOT_DIR := $(abspath ../..)
PROJECT_ROOT := $(CURDIR)
TARGET_NAME := $(notdir $(PROJECT_ROOT))
BUILD_DIR := $(abspath ../build)
TARGET := $(BUILD_DIR)/$(TARGET_NAME)
OLED_CONFIG_STAMP := $(BUILD_DIR)/.oled-driver
GAME_CONFIG_STAMP := $(BUILD_DIR)/.game-hub-config
FLASH_TOOL ?= ak-flash
FLASH_PORT ?= /dev/ttyUSB0
FLASH_ADDRESS ?= 0x08003000

SOC_NAME := STM32L151CBT6
SOC_FLASH_BYTES := 131072
SOC_SRAM_BYTES := 16384
FLASH_BASE_ADDRESS := 0x08000000
MEMORY_USAGE_BAR_WIDTH := 30

ARCH := cortex-m3
OS_CFG_SYSTICK_CLOCK_HZ ?= 32000000u
OS_CFG_HEAP_SIZE ?= 6144u
OS_CFG_USE_LOG ?= 0u
OLED_DRIVER ?= SH1106

include $(PROJECT_ROOT)/game_hub.cfg.mk

GAME_HUB_GAME_NAMES := TOWN_MAP BIKE_RACER MATH_RUSH HOLLOW UFO MAP3D \
	BLOCK_STACK DINO_RUNNER AUTO_SNAKE BITMAP_VIEWER

GAME_HUB_CONFIG_OPTIONS := $(addprefix GAME_HUB_ENABLE_,$(GAME_HUB_GAME_NAMES)) \
	GAME_HUB_ENABLE_DEBUG_PRINT

$(foreach option,$(GAME_HUB_CONFIG_OPTIONS),\
	$(if $(filter-out 0 1,$($(option))),\
		$(error $(option) must be 0 or 1))\
	$(if $(filter 1,$(words $($(option)))),,\
		$(error $(option) must be 0 or 1)))

GAME_HUB_CONFIG_FLAGS := $(foreach option,$(GAME_HUB_CONFIG_OPTIONS),\
	-D$(option)=$(strip $($(option))))
# Game mechanics tests cover every game; test-config exercises the release selection.
HOST_ALL_GAME_FLAGS := $(foreach game,$(GAME_HUB_GAME_NAMES),-DGAME_HUB_ENABLE_$(game)=1)
GAME_HUB_CONFIG_SIGNATURE := $(foreach option,$(GAME_HUB_CONFIG_OPTIONS),\
	$(option)=$(strip $($(option))))

ifeq ($(OLED_DRIVER),SSD1309)
HARDWARE_OPTION += -DSSD1309_DRIVER_EN -USH1106_DRIVER_EN
else ifeq ($(OLED_DRIVER),SH1106)
HARDWARE_OPTION += -DSH1106_DRIVER_EN -USSD1309_DRIVER_EN
else
$(error Unsupported OLED_DRIVER '$(OLED_DRIVER)'; use SSD1309 or SH1106)
endif

CPPFLAGS += $(HARDWARE_OPTION)
CPPFLAGS += $(GAME_HUB_CONFIG_FLAGS)

include $(ROOT_DIR)/kernel/kernel.mk
include $(PROJECT_ROOT)/common/common.mk
include $(PROJECT_ROOT)/platform/platform.mk
include $(PROJECT_ROOT)/services/services.mk
include $(PROJECT_ROOT)/app/app.mk

TOOLCHAIN ?= arm-none-eabi-
HOSTCC ?= gcc
CC := $(TOOLCHAIN)gcc
CXX := $(TOOLCHAIN)g++
OBJCOPY := $(TOOLCHAIN)objcopy
SIZE := $(TOOLCHAIN)size

CPU_FLAGS := -mcpu=cortex-m3 -mthumb
CFLAGS += $(CPU_FLAGS) -std=c11 -Os -g3 -ffunction-sections \
	-fdata-sections -Wall -Wextra -MMD -MP
CXXFLAGS += $(CPU_FLAGS) -std=c++11 -Os -g3 -ffunction-sections \
	-fdata-sections -Wall -Wextra -fno-exceptions -fno-rtti \
	-fno-use-cxa-atexit -MMD -MP
ASFLAGS += $(CPU_FLAGS) -g3
LDFLAGS += $(CPU_FLAGS) $(KERNEL_LDFLAGS) -T$(LINKER_SCRIPT) \
	-Wl,--gc-sections -Wl,-Map=$(TARGET).map --specs=nano.specs \
	--specs=nosys.specs
LDLIBS += -lc -lm -lnosys

# Keep source paths and extensions so modules can safely share a filename.
SOURCE_PATHS := $(patsubst $(ROOT_DIR)/%,%,$(abspath $(SOURCES) $(SOURCES_CPP)))
OBJECTS := $(addprefix $(BUILD_DIR)/obj/,$(addsuffix .o,$(SOURCE_PATHS)))
DEPS := $(OBJECTS:.o=.d)
HOST_GAME_CONFIG_SOURCES := $(filter app/games/%.c,$(SOURCES))
HOST_GAME_CONFIG_SOURCES := $(filter-out app/games/hollow/hollow_audio.c,$(HOST_GAME_CONFIG_SOURCES))
HOST_GAME_CONFIG_INCLUDES := $(filter -I%,$(CPPFLAGS))

.DEFAULT_GOAL := all

.PHONY: all clean flash size test test-config format format-check FORCE

all: $(TARGET).elf $(TARGET).hex $(TARGET).bin
	$(SIZE) $(TARGET).elf
	@printf '\nFLASH COMMAND:\n'
	@printf 'make flash FLASH_PORT=%s\n\n' '$(FLASH_PORT)'

flash: $(TARGET).bin
	cd $(BUILD_DIR) && $(FLASH_TOOL) $(FLASH_PORT) $(TARGET_NAME).bin $(FLASH_ADDRESS)

size: $(TARGET).elf
	@FLASH_USED=`$(SIZE) -A -d $(TARGET).elf | awk \
		'$$1==".isr_vector" || $$1==".text" || $$1==".init_array" || \
		 $$1==".fini_array" || $$1==".task_desc" || $$1==".ARM.exidx" || \
		 $$1==".data" {sum += $$2} END {print sum+0}'`; \
	RAM_USED=`$(SIZE) -A -d $(TARGET).elf | awk \
		'$$1==".data" || $$1==".bss" || $$1==".stack" || \
		 $$1==".heap" || $$1==".non_clear_ram" \
		 {sum += $$2} END {print sum+0}'`; \
	BOOT_RESERVED=$$(( $(FLASH_ADDRESS) - $(FLASH_BASE_ADDRESS) )); \
	APP_FLASH=$$(( $(SOC_FLASH_BYTES) - $$BOOT_RESERVED )); \
	FLASH_FREE=$$(( $$APP_FLASH - $$FLASH_USED )); \
	RAM_FREE=$$(( $(SOC_SRAM_BYTES) - $$RAM_USED )); \
	BOOT_END=$$(( $(FLASH_ADDRESS) - 1 )); \
	FLASH_END=$$(( $(FLASH_BASE_ADDRESS) + $(SOC_FLASH_BYTES) - 1 )); \
	FLASH_PCT=`awk "BEGIN {printf \"%.2f\", $$FLASH_USED * 100.0 / $$APP_FLASH}"`; \
	FLASH_FREE_PCT=`awk "BEGIN {printf \"%.2f\", $$FLASH_FREE * 100.0 / $$APP_FLASH}"`; \
	RAM_PCT=`awk "BEGIN {printf \"%.2f\", $$RAM_USED * 100.0 / $(SOC_SRAM_BYTES)}"`; \
	RAM_FREE_PCT=`awk "BEGIN {printf \"%.2f\", $$RAM_FREE * 100.0 / $(SOC_SRAM_BYTES)}"`; \
	FLASH_BLOCKS=$$(( $$FLASH_USED * $(MEMORY_USAGE_BAR_WIDTH) / $$APP_FLASH )); \
	RAM_BLOCKS=$$(( $$RAM_USED * $(MEMORY_USAGE_BAR_WIDTH) / $(SOC_SRAM_BYTES) )); \
	FLASH_BAR=`printf "%*s" $$FLASH_BLOCKS | tr ' ' '#'`; \
	FLASH_GAP=`printf "%*s" $$(( $(MEMORY_USAGE_BAR_WIDTH) - $$FLASH_BLOCKS )) | tr ' ' '-'`; \
	RAM_BAR=`printf "%*s" $$RAM_BLOCKS | tr ' ' '#'`; \
	RAM_GAP=`printf "%*s" $$(( $(MEMORY_USAGE_BAR_WIDTH) - $$RAM_BLOCKS )) | tr ' ' '-'`; \
	printf 'Memory usage for %s\n' '$(TARGET).elf'; \
	printf 'MCU name              : %s\n' '$(SOC_NAME)'; \
	printf 'Boot/Reserved Flash   : 0x%08X - 0x%08X (%d bytes)\n' \
		$(FLASH_BASE_ADDRESS) $$BOOT_END $$BOOT_RESERVED; \
	printf 'Application Flash     : 0x%08X - 0x%08X (%d bytes)\n' \
		$(FLASH_ADDRESS) $$FLASH_END $$APP_FLASH; \
	printf 'Flash                 : %d / %d bytes (%d bytes free)\n' \
		$$FLASH_USED $$APP_FLASH $$FLASH_FREE; \
	printf 'Flash usage           : [%s%s] %s%% used / %s%% free\n' \
		"$$FLASH_BAR" "$$FLASH_GAP" "$$FLASH_PCT" "$$FLASH_FREE_PCT"; \
	printf 'RAM                   : %d / %d bytes (%d bytes free)\n' \
		$$RAM_USED $(SOC_SRAM_BYTES) $$RAM_FREE; \
	printf 'RAM usage             : [%s%s] %s%% used / %s%% free\n' \
		"$$RAM_BAR" "$$RAM_GAP" "$$RAM_PCT" "$$RAM_FREE_PCT"
	@printf '\nGNU size summary:\n'
	$(SIZE) $(TARGET).elf
	@printf '\nSection details:\n'
	$(SIZE) -A -x $(TARGET).elf

FORCE:

$(OLED_CONFIG_STAMP): FORCE
	@mkdir -p $(dir $@)
	@if test "$$(cat $@ 2>/dev/null)" != "$(OLED_DRIVER)"; then \
		printf '%s\n' '$(OLED_DRIVER)' > $@; \
	fi

$(GAME_CONFIG_STAMP): FORCE
	@mkdir -p $(dir $@)
	@if test "$$(cat $@ 2>/dev/null)" != "$(GAME_HUB_CONFIG_SIGNATURE)"; then \
		printf '%s\n' '$(GAME_HUB_CONFIG_SIGNATURE)' > $@; \
	fi

$(OBJECTS): $(OLED_CONFIG_STAMP) $(GAME_CONFIG_STAMP)

$(TARGET).elf: $(OBJECTS) $(LINKER_SCRIPT) $(GAME_CONFIG_STAMP)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/obj/%.c.o: $(ROOT_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/obj/%.s.o: $(ROOT_DIR)/%.s
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/obj/%.cpp.o: $(ROOT_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

test: test-config
	@mkdir -p $(BUILD_DIR)
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror $(HOST_ALL_GAME_FLAGS) \
		-Iservices/display -Iservices/input -Iservices/audio -Iplatform/buzzer \
		-Iapp -Iapp/games/hollow -Iplatform \
		-Iapp/screen_manager -Iapp/games/math_rush \
		-Iapp/games/bike_racer -Iapp/games/ufo \
		-Iapp/games/map3d -Iapp/games/block_stack \
		-Iapp/games/dino_runner -Iapp/games/auto_snake \
		-Iapp/games/bitmap_viewer \
		-Iapp/games/town_map -Iapp/games/town_map/farm \
		-Iapp/games/town_map/fish_pond \
		tests/test_hollow_game.c app/games/hollow/hollow_game.c \
		app/games/hollow/hollow_bitmap.c \
		app/screen_manager/game_registry.c \
		app/screen_manager/game_menu.c \
		app/screen_manager/game_menu_icons.c \
		app/screen_manager/game_manager.c \
		app/games/math_rush/math_rush_game.c \
		app/games/math_rush/math_rush_question.c -lm \
		app/games/bike_racer/bike_racer_game.c \
		app/games/bike_racer/bike_racer_bitmap.c \
		app/games/ufo/ufo_game.c \
		app/games/ufo/ufo_bitmap.c \
		app/games/map3d/map3d_game.c \
		app/games/block_stack/block_stack_game.c \
		app/games/dino_runner/dino_bitmap.c \
		app/games/dino_runner/dino_game.c \
		app/games/auto_snake/auto_snake_game.c \
		app/games/bitmap_viewer/bitmap_viewer_bitmaps.c \
		app/games/bitmap_viewer/bitmap_viewer_game.c \
		app/games/town_map/town_map_game.c \
		app/games/town_map/town_areas.c \
		app/games/town_map/town_area_bitmaps.c \
		app/games/town_map/farm/town_farm.c \
		app/games/town_map/fish_pond/town_fish_pond.c \
		app/games/town_map/player_bitmap.c \
		app/games/town_map/town_player_assets.c \
		services/audio/audio.c \
		-o $(BUILD_DIR)/test_hollow_game
	$(BUILD_DIR)/test_hollow_game
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror \
		-Iservices/display -Iservices/input -Iservices/audio -Iplatform/buzzer \
		-Iapp -Iapp/games/math_rush -Iplatform \
		tests/test_math_rush.c app/games/math_rush/math_rush_game.c \
		app/games/math_rush/math_rush_question.c services/audio/audio.c \
		-o $(BUILD_DIR)/test_math_rush
	$(BUILD_DIR)/test_math_rush
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror \
		-Iservices/display -Iservices/input -Iservices/audio -Iplatform/buzzer \
		-Iapp -Iapp/games/auto_snake -Iplatform \
		tests/test_auto_snake.c -o $(BUILD_DIR)/test_auto_snake
	$(BUILD_DIR)/test_auto_snake
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror \
		-Iservices/display -Iservices/input -Iservices/audio -Iplatform/buzzer \
		-Iapp -Iapp/games/town_map -Iapp/games/town_map/farm \
		-Iapp/games/town_map/fish_pond \
		-Iapp/games/bitmap_viewer -Iplatform \
		tests/test_town_map.c app/games/town_map/town_map_game.c \
		app/games/town_map/town_areas.c \
		app/games/town_map/town_area_bitmaps.c \
		app/games/town_map/farm/town_farm.c \
		app/games/town_map/fish_pond/town_fish_pond.c \
		app/games/town_map/player_bitmap.c \
		app/games/town_map/town_player_assets.c \
		app/games/bitmap_viewer/bitmap_viewer_bitmaps.c \
		-o $(BUILD_DIR)/test_town_map
	$(BUILD_DIR)/test_town_map
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror \
		-Iservices/display tests/test_graphics.c services/display/graphics.c \
		-o $(BUILD_DIR)/test_graphics
	$(BUILD_DIR)/test_graphics
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror \
		-Iservices/display -Iplatform/oled \
		tests/test_display.c services/display/display.c services/display/graphics.c \
		-o $(BUILD_DIR)/test_display
	$(BUILD_DIR)/test_display
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror \
		-Iservices/input -Iplatform \
		tests/test_game_input.c services/input/game_input.c \
		-o $(BUILD_DIR)/test_game_input
	$(BUILD_DIR)/test_game_input
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror \
		-Iservices/audio -Iplatform/buzzer -Iapp/games/hollow \
		tests/test_audio.c services/audio/audio.c services/audio/buzzer_music.c \
		app/games/hollow/hollow_audio.c -o $(BUILD_DIR)/test_audio
	$(BUILD_DIR)/test_audio

test-config:
	@mkdir -p $(BUILD_DIR)
	$(HOSTCC) -std=c11 -Wall -Wextra -Werror $(GAME_HUB_CONFIG_FLAGS) \
		$(HOST_GAME_CONFIG_INCLUDES) tests/test_game_config.c \
		app/screen_manager/game_registry.c app/screen_manager/game_manager.c \
		app/screen_manager/game_menu.c app/screen_manager/game_menu_icons.c \
		$(HOST_GAME_CONFIG_SOURCES) services/audio/audio.c -lm -o $(BUILD_DIR)/test_game_config
	$(BUILD_DIR)/test_game_config

com: 
	@printf '\nOpen minicom with /dev/ttyUSB0\n'
	@sudo minicom -D /dev/ttyUSB0 -b 115200

-include $(DEPS)

format:
	python3 tools/format_source.py

format-check:
	python3 tools/format_source.py --check
