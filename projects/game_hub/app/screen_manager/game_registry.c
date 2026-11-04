/**
 ******************************************************************************
 * @file    game_registry.c
 * @brief   Game registry module
 *
 * @module  Game Hub - Screen Manager
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "game_registry.h"

#include "game_menu_icons.h"

#if GAME_HUB_ENABLE_HOLLOW
#include "hollow_game.h"
#include "hollow_audio.h"
#endif

#if GAME_HUB_ENABLE_MATH_RUSH
#include "math_rush_game.h"
#endif

#if GAME_HUB_ENABLE_BIKE_RACER
#include "bike_racer_game.h"
#endif

#if GAME_HUB_ENABLE_UFO
#include "ufo_bitmap.h"
#include "ufo_game.h"
#endif

#if GAME_HUB_ENABLE_MAP3D
#include "map3d_game.h"
#endif

#if GAME_HUB_ENABLE_BLOCK_STACK
#include "block_stack_game.h"
#endif

#if GAME_HUB_ENABLE_DINO_RUNNER
#include "dino_game.h"
#endif

#if GAME_HUB_ENABLE_AUTO_SNAKE
#include "auto_snake_game.h"
#endif

#if GAME_HUB_ENABLE_BITMAP_VIEWER
#include "bitmap_viewer_game.h"
#endif

#if GAME_HUB_ENABLE_TOWN_MAP
#include "town_map_game.h"
#endif

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
#if GAME_HUB_ENABLE_HOLLOW
static void game_hollow_init(void *context, uint32_t seed) {
    (void)seed;
    hollow_game_init((hollow_game_t *)context);
}

static void game_hollow_deinit(void *context) {
    (void)context;
    hollow_audio_stop();
}

static void game_hollow_update(void *context, uint8_t input_state, uint32_t elapsed_ms) {
    (void)elapsed_ms;
    hollow_game_update((hollow_game_t *)context, input_state);
}

static void game_hollow_draw(const void *context) {
    hollow_game_draw((const hollow_game_t *)context);
}

static uint8_t game_hollow_take_events(void *context) {
    return hollow_game_take_events((hollow_game_t *)context);
}

static bool game_hollow_wants_menu(const void *context) {
    return ((const hollow_game_t *)context)->return_to_menu;
}
#endif

#if GAME_HUB_ENABLE_MATH_RUSH
static bool game_math_wants_menu(const void *context) {
    return ((const math_rush_game_t *)context)->return_to_menu;
}
#endif

#if GAME_HUB_ENABLE_BIKE_RACER
static bool game_racer_wants_menu(const void *context) {
    return ((const bike_racer_game_t *)context)->return_to_menu;
}
#endif

#if GAME_HUB_ENABLE_UFO
static bool game_ufo_wants_menu(const void *context) {
    return ((const ufo_game_t *)context)->return_to_menu;
}
#endif

#if GAME_HUB_ENABLE_BLOCK_STACK
static bool game_block_stack_wants_menu(const void *context) {
    return ((const block_stack_game_t *)context)->return_to_menu;
}
#endif

#if GAME_HUB_ENABLE_DINO_RUNNER
static bool game_dino_wants_menu(const void *context) {
    return ((const dino_game_t *)context)->return_to_menu;
}
#endif

#if GAME_HUB_ENABLE_AUTO_SNAKE
static bool game_auto_snake_wants_menu(const void *context) {
    return ((const auto_snake_game_t *)context)->return_to_menu;
}
#endif

#if GAME_HUB_ENABLE_BITMAP_VIEWER
static bool game_bitmap_viewer_wants_menu(const void *context) {
    return ((const bitmap_viewer_game_t *)context)->return_to_menu;
}
#endif

#if GAME_HUB_ENABLE_TOWN_MAP
static bool game_town_map_wants_menu(const void *context) {
    return ((const town_map_game_t *)context)->return_to_menu;
}
#endif

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/

static const game_entry_t game_table[] = {
#if GAME_HUB_ENABLE_TOWN_MAP
    {"TOWN MAP", game_menu_icon_town_map, 16u, 16u, 2u, sizeof(town_map_game_t), town_map_game_init,
     town_map_game_deinit, town_map_game_update, town_map_game_draw, NULL,
     game_town_map_wants_menu},
#endif
#if GAME_HUB_ENABLE_BIKE_RACER
    {"BIKE RACER", game_menu_icon_racer, 16u, 16u, 2u, sizeof(bike_racer_game_t),
     bike_racer_game_init, bike_racer_game_deinit, bike_racer_game_update, bike_racer_game_draw,
     NULL, game_racer_wants_menu},
#endif
#if GAME_HUB_ENABLE_MATH_RUSH
    {"MATH RUSH", game_menu_icon_math, 16u, 16u, 2u, sizeof(math_rush_game_t), math_rush_game_init,
     math_rush_game_deinit, math_rush_game_update, math_rush_game_draw, NULL, game_math_wants_menu},
#endif
#if GAME_HUB_ENABLE_HOLLOW
    {"HOLLOW", game_menu_icon_hollow, 16u, 16u, 2u, sizeof(hollow_game_t), game_hollow_init,
     game_hollow_deinit, game_hollow_update, game_hollow_draw, game_hollow_take_events,
     game_hollow_wants_menu},
#endif
#if GAME_HUB_ENABLE_UFO
    {"UFO", ufo_player, 16u, 16u, 2u, sizeof(ufo_game_t), ufo_game_init, ufo_game_deinit,
     ufo_game_update, ufo_game_draw, NULL, game_ufo_wants_menu},
#endif
#if GAME_HUB_ENABLE_MAP3D
    {"3D MAP", game_menu_icon_map3d, 16u, 16u, 2u, sizeof(map3d_game_t), map3d_game_init,
     map3d_game_deinit, map3d_game_update, map3d_game_draw, NULL, NULL},
#endif
#if GAME_HUB_ENABLE_BLOCK_STACK
    {"BLOCK STACK", game_menu_icon_block_stack, 16u, 16u, 2u, sizeof(block_stack_game_t),
     block_stack_game_init, block_stack_game_deinit, block_stack_game_update, block_stack_game_draw,
     NULL, game_block_stack_wants_menu},
#endif
#if GAME_HUB_ENABLE_DINO_RUNNER
    {"T-REX", game_menu_icon_dino_runner, 16u, 16u, 2u, sizeof(dino_game_t), dino_game_init,
     dino_game_deinit, dino_game_update, dino_game_draw, NULL, game_dino_wants_menu},
#endif
#if GAME_HUB_ENABLE_AUTO_SNAKE
    {"AUTO SNAKE", game_menu_icon_auto_snake, 16u, 16u, 2u, sizeof(auto_snake_game_t),
     auto_snake_game_init, auto_snake_game_deinit, auto_snake_game_update, auto_snake_game_draw,
     NULL, game_auto_snake_wants_menu},
#endif
#if GAME_HUB_ENABLE_BITMAP_VIEWER
    {"BITMAP VIEWER", game_menu_icon_bitmap_viewer, 16u, 16u, 2u, sizeof(bitmap_viewer_game_t),
     bitmap_viewer_game_init, bitmap_viewer_game_deinit, bitmap_viewer_game_update,
     bitmap_viewer_game_draw, NULL, game_bitmap_viewer_wants_menu},
#endif
    {0}, /* Keep the array valid when every game is disabled. */
};

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
size_t game_registry_count(void) {
    return sizeof(game_table) / sizeof(game_table[0]) - 1u;
}

const game_entry_t *game_registry_get(size_t index) {
    if (index >= game_registry_count()) {
        return NULL;
    }
    return &game_table[index];
}
