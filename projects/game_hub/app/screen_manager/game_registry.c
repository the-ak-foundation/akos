#include "game_registry.h"

#include "hollow_game.h"
#include "hollow_audio.h"
#include "game_hal.h"
#include "game_menu_icons.h"
#include "bike_racer_game.h"
#include "block_stack_game.h"
#include "auto_snake_game.h"
#include "dino_game.h"
#include "kung_fu_game.h"
#include "math_rush_game.h"
#include "map3d_game.h"
#include "ufo_bitmap.h"
#include "ufo_game.h"

static void game_hollow_init(void *context, uint32_t seed) {
    (void)seed;
    hollow_game_init((hollow_game_t *)context);
}

static void game_hollow_deinit(void *context) {
    (void)context;
    hollow_audio_stop();
}

static void game_hollow_update(void *context, uint8_t input_state,
                               uint32_t elapsed_ms) {
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

static bool game_math_wants_menu(const void *context) {
    return ((const math_rush_game_t *)context)->return_to_menu;
}

static bool game_racer_wants_menu(const void *context) {
    return ((const bike_racer_game_t *)context)->return_to_menu;
}

static bool game_ufo_wants_menu(const void *context) {
    return ((const ufo_game_t *)context)->return_to_menu;
}

static bool game_block_stack_wants_menu(const void *context) {
    return ((const block_stack_game_t *)context)->return_to_menu;
}

static bool game_dino_wants_menu(const void *context) {
    return ((const dino_game_t *)context)->return_to_menu;
}

static bool game_auto_snake_wants_menu(const void *context) {
    return ((const auto_snake_game_t *)context)->return_to_menu;
}

static bool game_kung_fu_wants_menu(const void *context) {
    return ((const kung_fu_game_t *)context)->return_to_menu;
}

static const game_entry_t game_table[] = {
    {"BIKE RACER", game_menu_icon_racer, 16u, 16u, 2u,
     sizeof(bike_racer_game_t), bike_racer_game_init,
     bike_racer_game_deinit, bike_racer_game_update, bike_racer_game_draw,
     NULL, game_racer_wants_menu},
    {"MATH RUSH", game_menu_icon_math, 16u, 16u, 2u,
     sizeof(math_rush_game_t), math_rush_game_init,
     math_rush_game_deinit, math_rush_game_update, math_rush_game_draw, NULL,
     game_math_wants_menu},
    {"HOLLOW", game_menu_icon_hollow, 16u, 16u, 2u,
     sizeof(hollow_game_t), game_hollow_init, game_hollow_deinit,
     game_hollow_update, game_hollow_draw, game_hollow_take_events,
     game_hollow_wants_menu},
    {"UFO", ufo_player, 16u, 16u, 2u,
     sizeof(ufo_game_t),
     ufo_game_init, ufo_game_deinit,
     ufo_game_update, ufo_game_draw, NULL, game_ufo_wants_menu},
    {"3D MAP", game_menu_icon_map3d, 16u, 16u, 2u,
     sizeof(map3d_game_t), map3d_game_init, map3d_game_deinit,
     map3d_game_update, map3d_game_draw, NULL, NULL},
    {"BLOCK STACK", game_menu_icon_block_stack, 16u, 16u, 2u,
     sizeof(block_stack_game_t), block_stack_game_init,
     block_stack_game_deinit, block_stack_game_update,
     block_stack_game_draw, NULL, game_block_stack_wants_menu},
    {"T-REX", game_menu_icon_dino_runner, 16u, 16u, 2u,
     sizeof(dino_game_t), dino_game_init, dino_game_deinit,
     dino_game_update, dino_game_draw, NULL, game_dino_wants_menu},
    {"AUTO SNAKE", game_menu_icon_auto_snake, 16u, 16u, 2u,
     sizeof(auto_snake_game_t), auto_snake_game_init,
     auto_snake_game_deinit, auto_snake_game_update,
     auto_snake_game_draw, NULL, game_auto_snake_wants_menu},
    {"KUNG FU", game_menu_icon_kung_fu, 16u, 16u, 2u,
     sizeof(kung_fu_game_t), kung_fu_game_init,
     kung_fu_game_deinit, kung_fu_game_update,
     kung_fu_game_draw, NULL, game_kung_fu_wants_menu},
};

size_t game_registry_count(void) {
    return sizeof(game_table) / sizeof(game_table[0]);
}

const game_entry_t *game_registry_get(size_t index) {
    if (index >= game_registry_count()) {
        return NULL;
    }
    return &game_table[index];
}
