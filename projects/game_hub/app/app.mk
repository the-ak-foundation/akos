include ./app/screen_manager/screen_manager.mk
ifeq ($(strip $(GAME_HUB_ENABLE_HOLLOW)),1)
include ./app/games/hollow/hollow.mk
endif
ifeq ($(strip $(GAME_HUB_ENABLE_MATH_RUSH)),1)
include ./app/games/math_rush/math_rush.mk
endif
ifeq ($(strip $(GAME_HUB_ENABLE_BIKE_RACER)),1)
include ./app/games/bike_racer/bike_racer.mk
endif
ifeq ($(strip $(GAME_HUB_ENABLE_UFO)),1)
include ./app/games/ufo/ufo.mk
endif
ifeq ($(strip $(GAME_HUB_ENABLE_MAP3D)),1)
include ./app/games/map3d/map3d.mk
endif
ifeq ($(strip $(GAME_HUB_ENABLE_BLOCK_STACK)),1)
include ./app/games/block_stack/block_stack.mk
endif
ifeq ($(strip $(GAME_HUB_ENABLE_DINO_RUNNER)),1)
include ./app/games/dino_runner/dino_runner.mk
endif
ifeq ($(strip $(GAME_HUB_ENABLE_AUTO_SNAKE)),1)
include ./app/games/auto_snake/auto_snake.mk
endif
ifeq ($(strip $(GAME_HUB_ENABLE_BITMAP_VIEWER)),1)
include ./app/games/bitmap_viewer/bitmap_viewer.mk
endif
ifeq ($(strip $(GAME_HUB_ENABLE_TOWN_MAP)),1)
include ./app/games/town_map/town_map.mk
endif

# Town Map and Bitmap Viewer share the farm/catalog bitmaps, independently of
# whether the Bitmap Viewer game itself is included in the release.
ifneq ($(filter 1,$(GAME_HUB_ENABLE_TOWN_MAP) $(GAME_HUB_ENABLE_BITMAP_VIEWER)),)
CPPFLAGS += -I./app/games/bitmap_viewer
VPATH += app/games/bitmap_viewer
SOURCES += app/games/bitmap_viewer/bitmap_viewer_bitmaps.c
endif

include ./app/shell/shell.mk

CFLAGS += -I./app
CPPFLAGS += -I./app

VPATH += app

SOURCES += \
	app/app.c \
	app/task_game_hub.c \
	app/task_list.c \
	app/task_life.c
