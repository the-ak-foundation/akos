#ifndef __GAME_GFX_H__
#define __GAME_GFX_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void game_gfx_draw_text(uint8_t x, uint8_t y, const char *text,
                        uint8_t size, bool foreground, bool background);
void game_gfx_draw_number(uint8_t x, uint8_t y, uint32_t value,
                          uint8_t size, bool foreground, bool background);
void game_gfx_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                        bool on);

#ifdef __cplusplus
}
#endif

#endif /* __GAME_GFX_H__ */
