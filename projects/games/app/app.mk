include ./app/launcher/launcher.mk
include ./app/hollow/hollow.mk
include ./app/math_rush/math_rush.mk
include ./app/bike_racer/bike_racer.mk
include ./app/ufo/ufo.mk
include ./app/map3d/map3d.mk
include ./app/block_stack/block_stack.mk
include ./app/dino_runner/dino_runner.mk
include ./app/auto_snake/auto_snake.mk
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
