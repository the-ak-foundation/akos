################################################################################
# @file    game_hub.cfg.mk
# @brief   Configure Game Hub games and diagnostic output.
# @module  Game Hub - Configuration
################################################################################

# Diagnostic prints: 1 enables APP_DBG/APP_PRINT, 0 removes them from firmware.
# Shell prompts and command responses remain available when diagnostics are off.
GAME_HUB_ENABLE_DEBUG_PRINT ?= 1

GAME_HUB_ENABLE_BIKE_RACER    ?= 1
GAME_HUB_ENABLE_MATH_RUSH     ?= 1
GAME_HUB_ENABLE_HOLLOW        ?= 1
GAME_HUB_ENABLE_UFO           ?= 1
GAME_HUB_ENABLE_MAP3D         ?= 1
GAME_HUB_ENABLE_BLOCK_STACK   ?= 1
GAME_HUB_ENABLE_DINO_RUNNER   ?= 1

# Not ready
GAME_HUB_ENABLE_AUTO_SNAKE    ?= 1
GAME_HUB_ENABLE_TOWN_MAP      ?= 0

# For debug and dev
GAME_HUB_ENABLE_BITMAP_VIEWER ?= 0
