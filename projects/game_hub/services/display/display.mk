CPPFLAGS += -I./services/display -I./services/display/third_party/adafruit_gfx
VPATH += services/display services/display/third_party/adafruit_gfx

SOURCES += services/display/display.c services/display/graphics.c
SOURCES_CPP += services/display/graphics_text.cpp \
	services/display/third_party/adafruit_gfx/Adafruit_GFX.cpp
