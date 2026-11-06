/**
 ******************************************************************************
 * @file    graphics.c
 * @brief   Monochrome framebuffer drawing
 *
 * @module  Game Hub - Display
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "graphics.h"

#include <string.h>

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/
static uint8_t gfx_buffer[GFX_BUFFER_SIZE];

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void gfx_clear(void) {
    memset(gfx_buffer, 0, sizeof(gfx_buffer));
}

void gfx_draw_pixel(uint8_t x, uint8_t y, bool on) {
    uint8_t *pixel;
    uint8_t mask;

    if ((x >= GFX_WIDTH) || (y >= GFX_HEIGHT)) {
        return;
    }

    pixel = &gfx_buffer[((uint32_t)y / 8u) * GFX_WIDTH + x];
    mask = (uint8_t)(1u << (y & 7u));
    *pixel = on ? (uint8_t)(*pixel | mask) : (uint8_t)(*pixel & ~mask);
}

void gfx_draw_bitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height,
                     bool on) {
    if (bitmap == NULL) {
        return;
    }

    for (uint8_t row = 0u; row < height; row++) {
        for (uint8_t column = 0u; column < width; column++) {
            uint16_t draw_x = (uint16_t)x + column;
            uint16_t draw_y = (uint16_t)y + row;
            uint8_t data = bitmap[(uint32_t)(row / 8u) * width + column];

            if ((draw_x < GFX_WIDTH) && (draw_y < GFX_HEIGHT) &&
                ((data & (1u << (row & 7u))) != 0u)) {
                gfx_draw_pixel((uint8_t)draw_x, (uint8_t)draw_y, on);
            }
        }
    }
}

void gfx_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on) {
    for (uint16_t row = y; (row < (uint16_t)y + height) && (row < GFX_HEIGHT); row++) {
        for (uint16_t column = x; (column < (uint16_t)x + width) && (column < GFX_WIDTH);
             column++) {
            gfx_draw_pixel((uint8_t)column, (uint8_t)row, on);
        }
    }
}

const uint8_t *gfx_get_buffer(size_t *size) {
    if (size != NULL) {
        *size = sizeof(gfx_buffer);
    }
    return gfx_buffer;
}
