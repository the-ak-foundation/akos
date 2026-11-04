/**
 ******************************************************************************
 * @file    bitmap_viewer_game.c
 * @brief   Bitmap viewer game module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "bitmap_viewer_game.h"

#include "graphics.h"

#include <stddef.h>
#include <string.h>

#include "game_input.h"
#include "bitmap_viewer_bitmaps.h"

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void bitmap_viewer_draw(uint8_t x, uint8_t y, const bitmap_viewer_entry_t *entry) {
    for (uint8_t row = 0u; row < entry->height; row++) {
        for (uint8_t column = 0u; column < entry->width; column++) {
            uint8_t value = entry->data[(uint16_t)row * entry->stride + column / 8u];

            if ((value & (uint8_t)(0x80u >> (column & 7u))) != 0u) {
                gfx_draw_pixel((uint8_t)(x + column), (uint8_t)(y + row), true);
            }
        }
    }
}

static void bitmap_viewer_draw_farm_layout(void) {
    for (size_t index = 0u; index < bitmap_viewer_farm_layout_count; index++) {
        const bitmap_viewer_placement_t *placement = &bitmap_viewer_farm_layout[index];

        bitmap_viewer_draw(placement->x, placement->y, &placement->bitmap);
    }
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void bitmap_viewer_game_init(void *context, uint32_t seed) {
    bitmap_viewer_game_t *viewer = (bitmap_viewer_game_t *)context;

    (void)seed;
    if (viewer == NULL) {
        return;
    }
    memset(viewer, 0, sizeof(*viewer));
}

void bitmap_viewer_game_deinit(void *context) {
    (void)context;
}

void bitmap_viewer_game_update(void *context, uint8_t input_state, uint32_t elapsed_ms) {
    bitmap_viewer_game_t *viewer = (bitmap_viewer_game_t *)context;
    uint8_t pressed;

    (void)elapsed_ms;
    if (viewer == NULL) {
        return;
    }
    pressed = (uint8_t)(input_state & ~viewer->input_previous);
    viewer->input_previous = input_state;

    if ((pressed & GAME_INPUT_MODE) != 0u) {
        viewer->return_to_menu = true;
    } else if ((pressed & GAME_INPUT_UP) != 0u) {
        viewer->selected = (viewer->selected == 0u) ? (uint8_t)bitmap_viewer_catalog_count
                                                    : (uint8_t)(viewer->selected - 1u);
    } else if ((pressed & GAME_INPUT_DOWN) != 0u) {
        viewer->selected = (uint8_t)((viewer->selected + 1u) % (bitmap_viewer_catalog_count + 1u));
    }
}

void bitmap_viewer_game_draw(const void *context) {
    const bitmap_viewer_game_t *viewer = (const bitmap_viewer_game_t *)context;
    const bitmap_viewer_entry_t *entry;
    uint8_t number_x;
    uint8_t bitmap_x;
    uint8_t bitmap_y;

    if (viewer == NULL) {
        return;
    }
    entry = (viewer->selected == 0u) ? NULL : &bitmap_viewer_catalog[viewer->selected - 1u];
    number_x = (viewer->selected < 9u) ? 122u : 116u;

    gfx_clear();
    if (entry == NULL) {
        bitmap_viewer_draw_farm_layout();
    } else {
        bitmap_x = (uint8_t)((GFX_WIDTH - entry->width) / 2u);
        bitmap_y = (uint8_t)((GFX_HEIGHT - entry->height) / 2u);
        bitmap_viewer_draw(bitmap_x, bitmap_y, entry);
    }
    gfx_draw_number(number_x, 0u, (uint32_t)viewer->selected + 1u, 1u, true, false);
}
