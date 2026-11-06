CPPFLAGS += -I./platform/oled
CFLAGS += -I./platform/oled

VPATH += platform/oled platform/oled/i2c

SOURCES += \
	platform/oled/oled.c \
	platform/oled/i2c/oled_i2c.c
