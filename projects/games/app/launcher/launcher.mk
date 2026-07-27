CFLAGS += -I./app/launcher
CPPFLAGS += -I./app/launcher

VPATH += app/launcher

SOURCES += \
	app/launcher/game_registry.c \
	app/launcher/game_menu_icons.c \
	app/launcher/game_menu.c \
	app/launcher/game_manager.c
