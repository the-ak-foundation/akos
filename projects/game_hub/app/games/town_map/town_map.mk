CFLAGS += -I./app/games/town_map
CFLAGS += -I./app/games/town_map/farm
CFLAGS += -I./app/games/town_map/fish_pond
CPPFLAGS += -I./app/games/town_map
CPPFLAGS += -I./app/games/town_map/farm
CPPFLAGS += -I./app/games/town_map/fish_pond

VPATH += app/games/town_map
VPATH += app/games/town_map/farm
VPATH += app/games/town_map/fish_pond

SOURCES += \
	app/games/town_map/town_map_game.c \
	app/games/town_map/town_areas.c \
	app/games/town_map/town_area_bitmaps.c \
	app/games/town_map/farm/town_farm.c \
	app/games/town_map/fish_pond/town_fish_pond.c \
	app/games/town_map/player_bitmap.c \
	app/games/town_map/town_player_assets.c
