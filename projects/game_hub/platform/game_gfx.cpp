#include "game_gfx.h"

#include "Adafruit_GFX.h"
#include "game_hal.h"

class GameGfx : public Adafruit_GFX {
public:
    GameGfx() : Adafruit_GFX(GAME_DISPLAY_WIDTH, GAME_DISPLAY_HEIGHT) {
        setTextWrap(false);
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if ((x < 0) || (y < 0) ||
            (x >= (int16_t)GAME_DISPLAY_WIDTH) ||
            (y >= (int16_t)GAME_DISPLAY_HEIGHT)) {
            return;
        }
        game_display_draw_pixel((uint8_t)x, (uint8_t)y, color != 0u);
    }
};

static GameGfx game_gfx;

extern "C" void game_gfx_draw_text(uint8_t x, uint8_t y, const char *text,
                                   uint8_t size, bool foreground,
                                   bool background) {
    game_gfx.setCursor(x, y);
    game_gfx.setTextSize(size);
    game_gfx.setTextColor(foreground ? 1u : 0u, background ? 1u : 0u);
    game_gfx.print(text);
}

extern "C" void game_gfx_draw_number(uint8_t x, uint8_t y, uint32_t value,
                                     uint8_t size, bool foreground,
                                     bool background) {
    game_gfx.setCursor(x, y);
    game_gfx.setTextSize(size);
    game_gfx.setTextColor(foreground ? 1u : 0u, background ? 1u : 0u);
    game_gfx.print(value);
}

extern "C" void game_gfx_draw_rect(uint8_t x, uint8_t y, uint8_t width,
                                   uint8_t height, bool on) {
    game_gfx.drawRect(x, y, width, height, on ? 1u : 0u);
}
