include ./app/screen_manager/screen_manager.mk
include ./app/games/hollow/hollow.mk
include ./app/games/math_rush/math_rush.mk
include ./app/games/bike_racer/bike_racer.mk
include ./app/games/ufo/ufo.mk
include ./app/games/map3d/map3d.mk
include ./app/games/block_stack/block_stack.mk
include ./app/games/dino_runner/dino_runner.mk
include ./app/games/auto_snake/auto_snake.mk
include ./app/games/kung_fu_escape/kung_fu_escape.mk
include ./app/shell/shell.mk

CFLAGS += -I./app
CPPFLAGS += -I./app

VPATH += app

SOURCES += \
	app/app.c \
	app/game_hub_logo.c \
	app/task_game_hub.c \
	app/task_list.c \
	app/task_life.c
