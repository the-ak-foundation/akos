/**
 ******************************************************************************
 * @file    graphics.h
 * @brief   Graphics
 *
 * @module  Game Hub - Display
 ******************************************************************************
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
#define GFX_WIDTH       (128u)
#define GFX_HEIGHT      (64u)
#define GFX_BUFFER_SIZE (GFX_WIDTH * GFX_HEIGHT / 8u)

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void gfx_clear(void);
void gfx_draw_pixel(uint8_t x, uint8_t y, bool on);
void gfx_draw_bitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height,
                     bool on);
void gfx_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on);
/* Read-only page-major framebuffer: buffer[(y / 8) * GFX_WIDTH + x], bit y % 8. */
const uint8_t *gfx_get_buffer(size_t *size);
void gfx_draw_text(uint8_t x, uint8_t y, const char *text, uint8_t size, bool foreground,
                   bool background);
void gfx_draw_number(uint8_t x, uint8_t y, uint32_t value, uint8_t size, bool foreground,
                     bool background);
void gfx_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on);

#ifdef __cplusplus
}
#endif

#endif
