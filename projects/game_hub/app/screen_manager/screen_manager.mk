CFLAGS += -I./app/screen_manager
CPPFLAGS += -I./app/screen_manager

VPATH += app/screen_manager

SOURCES += \
	app/screen_manager/game_hub_logo.c \
	app/screen_manager/game_registry.c \
	app/screen_manager/game_menu_icons.c \
	app/screen_manager/game_menu.c \
	app/screen_manager/game_manager.c
