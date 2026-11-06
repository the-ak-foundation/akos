################################################################################
# @file    game_config.mk
# @brief   Load and validate shared Game Hub build configuration.
################################################################################

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
