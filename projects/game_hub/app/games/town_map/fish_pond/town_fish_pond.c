/**
 ******************************************************************************
 * @file    town_fish_pond.c
 * @brief   Town fish pond module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "town_fish_pond.h"

#include "graphics.h"

#include <stddef.h>

#include "game_input.h"
#include "fish_pond_bitmaps.h"
#include "town_areas.h"
#include "town_farm.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define FISH_POND_EXIT_HOLD_MS (1500u)

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void town_fish_pond_update(town_map_game_t *game, uint8_t input_state, uint8_t pressed,
                           uint32_t elapsed_ms) {
    const fish_pond_catalog_entry_t *fish;

    if ((pressed & GAME_INPUT_UP) != 0u) {
        game->fish_selected = (game->fish_selected == 0u) ? (FISH_POND_TYPE_COUNT - 1u)
                                                          : (uint8_t)(game->fish_selected - 1u);
    }
    if ((pressed & GAME_INPUT_DOWN) != 0u) {
        game->fish_selected = (uint8_t)((game->fish_selected + 1u) % FISH_POND_TYPE_COUNT);
    }
    fish = &fish_pond_catalog[game->fish_selected];
    if (((pressed & GAME_INPUT_MODE) != 0u) && (fish->bitmap != NULL) &&
        (game->coin_count >= fish->price) && (game->fish_owned[game->fish_selected] < UINT8_MAX)) {
        game->coin_count = (uint8_t)(game->coin_count - fish->price);
        game->fish_owned[game->fish_selected]++;
    }

    if ((input_state & GAME_INPUT_MODE) != 0u) {
        if (game->mode_hold_ms < FISH_POND_EXIT_HOLD_MS) {
            game->mode_hold_ms += elapsed_ms;
            if (game->mode_hold_ms >= FISH_POND_EXIT_HOLD_MS) {
                town_area_leave_fish_pond(game);
            }
        }
    } else {
        game->mode_hold_ms = 0u;
    }
}

void town_fish_pond_draw(const town_map_game_t *game) {
    const fish_pond_catalog_entry_t *fish = &fish_pond_catalog[game->fish_selected];

    gfx_clear();
    gfx_draw_text(2u, 1u, "FISH", 1u, true, false);
    gfx_draw_number(31u, 1u, (uint32_t)game->fish_selected + 1u, 1u, true, false);
    gfx_draw_text(38u, 1u, "/8", 1u, true, false);
    if (fish->bitmap != NULL) {
        gfx_draw_bitmap(32u, 11u, fish->bitmap, fish->width, fish->height, true);
        gfx_draw_text(50u, 44u, "BUY", 1u, true, false);
        gfx_draw_number(75u, 44u, fish->price, 1u, true, false);
    } else {
        gfx_draw_rect(32u, 11u, 64u, 32u, true);
        gfx_draw_text(48u, 23u, "TODO", 1u, true, false);
    }
    town_farm_draw_coin_counter(game, 2u, 54u);
    gfx_draw_text(78u, 54u, "OWN", 1u, true, false);
    gfx_draw_number(105u, 54u, game->fish_owned[game->fish_selected], 1u, true, false);
}
