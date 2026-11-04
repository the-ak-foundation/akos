/**
 ******************************************************************************
 * @file    graphics_text.cpp
 * @brief   Graphics text bridge
 *
 * @module  Game Hub - Display
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "graphics.h"

#include "Adafruit_GFX.h"

class Graphics : public Adafruit_GFX {
  public:
    Graphics() : Adafruit_GFX(GFX_WIDTH, GFX_HEIGHT) {
        setTextWrap(false);
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if ((x < 0) || (y < 0) || (x >= (int16_t)GFX_WIDTH) || (y >= (int16_t)GFX_HEIGHT)) {
            return;
        }
        gfx_draw_pixel((uint8_t)x, (uint8_t)y, color != 0u);
    }
};

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/

static Graphics graphics_text;

extern "C" void gfx_draw_text(uint8_t x, uint8_t y, const char *text, uint8_t size, bool foreground,
                              bool background) {
    graphics_text.setCursor(x, y);
    graphics_text.setTextSize(size);
    graphics_text.setTextColor(foreground ? 1u : 0u, background ? 1u : 0u);
    graphics_text.print(text);
}

extern "C" void gfx_draw_number(uint8_t x, uint8_t y, uint32_t value, uint8_t size, bool foreground,
                                bool background) {
    graphics_text.setCursor(x, y);
    graphics_text.setTextSize(size);
    graphics_text.setTextColor(foreground ? 1u : 0u, background ? 1u : 0u);
    graphics_text.print(value);
}

extern "C" void gfx_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on) {
    graphics_text.drawRect(x, y, width, height, on ? 1u : 0u);
}
