/**
 ******************************************************************************
 * @file    town_map_game.c
 * @brief   Town map game module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "town_map_game.h"

#include "graphics.h"

#include <stddef.h>
#include <string.h>

#include "game_input.h"
#include "player_bitmap.h"
#include "town_areas.h"
#include "town_farm.h"
#include "town_fish_pond.h"
#include "town_player_assets.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define TOWN_VERTICAL_X     ((TOWN_WORLD_WIDTH - TOWN_ROAD_WIDTH) / 2)
#define TOWN_HORIZONTAL_Y   ((TOWN_WORLD_HEIGHT - TOWN_ROAD_WIDTH) / 2)
#define TOWN_MOVE_PERIOD_MS (40u)
#define TOWN_ANIMATION_MS   (180u)
#define TOWN_MODE_EXIT_MS   (1500u)
#define TOWN_TILE_WIDTH     (4)
#define TOWN_TILE_HEIGHT    (4)

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/

static const int8_t town_direction_to_x[4] = {0, 1, 0, -1};
static const int8_t town_direction_to_y[4] = {-1, 0, 1, 0};

static const uint8_t town_walk_action[4] = {0u, 1u, 0u, 2u};
/* ArduSoccer source order: south, east, west, north. */
static const uint8_t town_direction_frame_base[4] = {9u, 3u, 0u, 6u};
static const PlayerDirection town_new_player_direction[4] = {
    PLAYER_DIR_UP,
    PLAYER_DIR_RIGHT,
    PLAYER_DIR_DOWN,
    PLAYER_DIR_LEFT,
};

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
bool town_world_player_fits(int16_t x, int16_t y) {
    return town_area_player_fits(TOWN_SCENE_TOWN, x, y);
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static int16_t town_clamp(int16_t value, int16_t low, int16_t high) {
    if (value < low) {
        return low;
    }
    return (value > high) ? high : value;
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
int16_t town_camera_x(const town_map_game_t *game) {
    int16_t center = (int16_t)(game->player.x + TOWN_PLAYER_WIDTH / 2);

    return town_clamp((int16_t)(center - GFX_WIDTH / 2), 0,
                      (int16_t)(TOWN_WORLD_WIDTH - GFX_WIDTH));
}

int16_t town_camera_y(const town_map_game_t *game) {
    int16_t center = (int16_t)(game->player.y + TOWN_PLAYER_HEIGHT / 2);

    return town_clamp((int16_t)(center - GFX_HEIGHT / 2), 0,
                      (int16_t)(TOWN_WORLD_HEIGHT - GFX_HEIGHT));
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void town_player_turn(town_player_t *player, int8_t amount) {
    player->direction =
        (town_direction_t)(((uint8_t)player->direction + 4u + (uint8_t)amount) & 3u);
}

static bool town_player_step(town_map_game_t *game) {
    town_player_t *player = &game->player;
    int16_t next_x = (int16_t)(player->x + town_direction_to_x[player->direction]);
    int16_t next_y = (int16_t)(player->y + town_direction_to_y[player->direction]);

    if (!town_area_player_fits(game->scene, next_x, next_y)) {
        player->state = TOWN_PLAYER_STANDING;
        player->animation_frame = 0u;
        return false;
    }
    player->x = next_x;
    player->y = next_y;
    (void)town_area_check_transition(game);
    return true;
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void town_map_game_init(void *context, uint32_t seed) {
    town_map_game_t *game = (town_map_game_t *)context;

    if (game == NULL) {
        return;
    }
    memset(game, 0, sizeof(*game));
    game->player.x = (int16_t)(TOWN_WORLD_WIDTH / 2 - TOWN_PLAYER_WIDTH / 2);
    game->player.y = (int16_t)(TOWN_WORLD_HEIGHT / 2 - TOWN_PLAYER_HEIGHT / 2);
    game->player.direction = TOWN_DIR_UP;
    game->player.state = TOWN_PLAYER_STANDING;
    game->scene = TOWN_SCENE_TOWN;
    game->character = TOWN_CHARACTER_SOCCER;
    game->selecting_character = true;
    game->farm_random_state = (seed == 0u) ? 0x6D2B79F5u : seed;
    game->coin_count = 2u;
}

void town_map_game_deinit(void *context) {
    (void)context;
}

void town_map_game_update(void *context, uint8_t input_state, uint32_t elapsed_ms) {
    town_map_game_t *game = (town_map_game_t *)context;
    uint8_t pressed;

    if (game == NULL) {
        return;
    }
    pressed = (uint8_t)(input_state & ~game->input_previous);
    game->input_previous = input_state;

    if (game->selecting_character) {
        if ((pressed & GAME_INPUT_UP) != 0u) {
            game->selected_character =
                (uint8_t)((game->selected_character + TOWN_CHARACTER_COUNT - 1u) %
                          TOWN_CHARACTER_COUNT);
        }
        if ((pressed & GAME_INPUT_DOWN) != 0u) {
            game->selected_character =
                (uint8_t)((game->selected_character + 1u) % TOWN_CHARACTER_COUNT);
        }
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game->character = (town_character_t)game->selected_character;
            game->selecting_character = false;
        }
        return;
    }

    if (game->area_intro_ms > 0u) {
        game->area_intro_ms =
            (elapsed_ms >= game->area_intro_ms) ? 0u : (uint16_t)(game->area_intro_ms - elapsed_ms);
        return;
    }

    if (game->scene == TOWN_SCENE_FARM) {
        town_farm_update(game, input_state, pressed, elapsed_ms);
        return;
    }
    if (game->scene == TOWN_SCENE_FISH_POND) {
        town_fish_pond_update(game, input_state, pressed, elapsed_ms);
        return;
    }

    if ((pressed & GAME_INPUT_UP) != 0u) {
        town_player_turn(&game->player, 1);
    }
    if ((pressed & GAME_INPUT_DOWN) != 0u) {
        town_player_turn(&game->player, -1);
    }
    if ((pressed & GAME_INPUT_MODE) != 0u) {
        game->player.state = (game->player.state == TOWN_PLAYER_WALKING) ? TOWN_PLAYER_STANDING
                                                                         : TOWN_PLAYER_WALKING;
        game->move_accumulator_ms = 0u;
    }

    if ((input_state & GAME_INPUT_MODE) != 0u) {
        if (game->mode_hold_ms < TOWN_MODE_EXIT_MS) {
            game->mode_hold_ms += elapsed_ms;
            if (game->mode_hold_ms >= TOWN_MODE_EXIT_MS) {
                game->return_to_menu = true;
            }
        }
    } else {
        game->mode_hold_ms = 0u;
    }

    if (game->player.state != TOWN_PLAYER_WALKING) {
        game->player.animation_frame = 0u;
        game->animation_accumulator_ms = 0u;
        return;
    }

    game->move_accumulator_ms += elapsed_ms;
    while (game->move_accumulator_ms >= TOWN_MOVE_PERIOD_MS) {
        game->move_accumulator_ms -= TOWN_MOVE_PERIOD_MS;
        if (!town_player_step(game)) {
            break;
        }
    }
    game->animation_accumulator_ms += elapsed_ms;
    if (game->animation_accumulator_ms >= TOWN_ANIMATION_MS) {
        game->animation_accumulator_ms %= TOWN_ANIMATION_MS;
        game->player.animation_frame = (uint8_t)((game->player.animation_frame + 1u) & 3u);
    }
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void town_draw_world_pixel(int16_t world_x, int16_t world_y, int16_t camera_x,
                                  int16_t camera_y) {
    int16_t screen_x = (int16_t)(world_x - camera_x);
    int16_t screen_y = (int16_t)(world_y - camera_y);

    if ((screen_x >= 0) && (screen_x < (int16_t)GFX_WIDTH) && (screen_y >= 0) &&
        (screen_y < (int16_t)GFX_HEIGHT)) {
        gfx_draw_pixel((uint8_t)screen_x, (uint8_t)screen_y, true);
    }
}

static bool town_ground_detail_on(int16_t x, int16_t y) {
    uint16_t cell_x = (uint16_t)x / 16u;
    uint16_t cell_y = (uint16_t)y / 8u;
    uint8_t local_x = (uint8_t)((uint16_t)x & 15u);
    uint8_t local_y = (uint8_t)((uint16_t)y & 7u);
    uint32_t hash = (uint32_t)cell_x * 0x45D9F3Bu ^ (uint32_t)cell_y * 0x119DE1F3u;

    hash ^= hash >> 16u;
    if ((hash % 17u) == 0u) {
        /* Small rock: six pixels in a low 4x3 silhouette. */
        return ((local_y == 4u) && ((local_x == 6u) || (local_x == 7u))) ||
               ((local_y == 5u) && ((local_x == 5u) || (local_x == 8u))) ||
               ((local_y == 6u) && ((local_x == 6u) || (local_x == 7u)));
    }
    if ((hash % 9u) == 0u) {
        /* Exact 11x5 silhouette of FARM's ui_grass_tuft_bitmap. */
        uint8_t grass_x = (uint8_t)(local_x - 2u);

        if ((local_x < 2u) || (local_x >= 13u) || (local_y >= 5u)) {
            return false;
        }
        return ((local_y == 0u) && ((grass_x == 2u) || (grass_x == 7u))) ||
               ((local_y == 1u) && ((grass_x == 3u) || (grass_x == 6u))) ||
               ((local_y == 2u) &&
                ((grass_x == 0u) || (grass_x == 4u) || (grass_x == 7u) || (grass_x == 10u))) ||
               ((local_y == 3u) &&
                ((grass_x == 1u) || (grass_x == 4u) || (grass_x == 6u) || (grass_x == 9u))) ||
               ((local_y == 4u) &&
                ((grass_x == 2u) || (grass_x == 4u) || (grass_x == 6u) || (grass_x == 8u)));
    }
    return false;
}

static void town_draw_road(const town_map_game_t *game) {
    int16_t camera_x = town_camera_x(game);
    int16_t camera_y = town_camera_y(game);
    int16_t end_x = (int16_t)(camera_x + GFX_WIDTH);
    int16_t end_y = (int16_t)(camera_y + GFX_HEIGHT);

    for (int16_t y = camera_y; y < end_y; y++) {
        for (int16_t x = camera_x; x < end_x; x++) {
            bool road = town_area_town_point_on_road(x, y);
            bool edge = road && (!town_area_town_point_on_road((int16_t)(x - 1), y) ||
                                 !town_area_town_point_on_road((int16_t)(x + 1), y) ||
                                 !town_area_town_point_on_road(x, (int16_t)(y - 1)) ||
                                 !town_area_town_point_on_road(x, (int16_t)(y + 1)));
            int16_t stagger = ((y / TOWN_TILE_HEIGHT) & 1) ? TOWN_TILE_WIDTH / 2 : 0;
            int16_t tile_x = (int16_t)((x + stagger) % TOWN_TILE_WIDTH);
            int16_t tile_y = (int16_t)(y % TOWN_TILE_HEIGHT);
            bool joint = road && ((tile_x == 0) || (tile_y == 0));
            bool ground_detail = !road && town_ground_detail_on(x, y);

            if (edge || joint || ground_detail) {
                town_draw_world_pixel(x, y, camera_x, camera_y);
            }
        }
    }
}

static void town_draw_soccer_player(int16_t screen_x, int16_t screen_y, town_direction_t direction,
                                    uint8_t action_frame, town_player_state_t state) {
    action_frame = (state == TOWN_PLAYER_WALKING) ? action_frame : 0u;
    uint8_t pose = town_walk_action[action_frame];
    uint8_t frame = (uint8_t)(town_direction_frame_base[direction] + pose);

    for (uint8_t row = 0u; row < TOWN_PLAYER_HEIGHT; row++) {
        for (uint8_t column = 0u; column < TOWN_PLAYER_WIDTH; column++) {
            uint8_t index = (uint8_t)((row / 8u) * TOWN_PLAYER_WIDTH + column);
            uint8_t bit = (uint8_t)(1u << (row & 7u));

            if ((town_soccer_player_mask[frame][index] & bit) != 0u) {
                gfx_draw_pixel((uint8_t)(screen_x + column), (uint8_t)(screen_y + row), false);
            }
        }
    }
    for (uint8_t row = 0u; row < TOWN_PLAYER_HEIGHT; row++) {
        for (uint8_t column = 0u; column < TOWN_PLAYER_WIDTH; column++) {
            uint8_t index = (uint8_t)((row / 8u) * TOWN_PLAYER_WIDTH + column);
            uint8_t bit = (uint8_t)(1u << (row & 7u));

            if ((town_soccer_player[frame][index] & bit) != 0u) {
                gfx_draw_pixel((uint8_t)(screen_x + column), (uint8_t)(screen_y + row), true);
            }
        }
    }
}

static bool town_new_player_pixel(const uint8_t *bitmap, int8_t x, int8_t y) {
    uint8_t index;
    uint8_t bit;

    if ((x < 0) || (x >= (int8_t)PLAYER_SPRITE_WIDTH) || (y < 0) ||
        (y >= (int8_t)PLAYER_SPRITE_HEIGHT)) {
        return false;
    }
    index = (uint8_t)((uint8_t)y * 2u + (uint8_t)x / 8u);
    bit = (uint8_t)(0x80u >> ((uint8_t)x & 7u));
    return (bitmap[index] & bit) != 0u;
}

static void town_draw_new_player(int16_t screen_x, int16_t screen_y, town_direction_t direction,
                                 uint8_t animation_frame) {
    const uint8_t *bitmap = player_get_bitmap(town_new_player_direction[direction],
                                              (PlayerFrame)(animation_frame & 1u));

    /* Clear a one-pixel silhouette around the player before drawing it. */
    for (int8_t row = 0; row < (int8_t)PLAYER_SPRITE_HEIGHT; row++) {
        for (int8_t column = 0; column < (int8_t)PLAYER_SPRITE_WIDTH; column++) {
            bool outline = !town_new_player_pixel(bitmap, column, row) &&
                           (town_new_player_pixel(bitmap, (int8_t)(column - 1), row) ||
                            town_new_player_pixel(bitmap, (int8_t)(column + 1), row) ||
                            town_new_player_pixel(bitmap, column, (int8_t)(row - 1)) ||
                            town_new_player_pixel(bitmap, column, (int8_t)(row + 1)));

            if (outline) {
                gfx_draw_pixel((uint8_t)(screen_x + column), (uint8_t)(screen_y + row), false);
            }
        }
    }
    for (int8_t row = 0; row < (int8_t)PLAYER_SPRITE_HEIGHT; row++) {
        for (int8_t column = 0; column < (int8_t)PLAYER_SPRITE_WIDTH; column++) {
            if (town_new_player_pixel(bitmap, column, row)) {
                gfx_draw_pixel((uint8_t)(screen_x + column), (uint8_t)(screen_y + row), true);
            }
        }
    }
}

static void town_draw_player(const town_map_game_t *game) {
    int16_t screen_x = (int16_t)(game->player.x - town_camera_x(game));
    int16_t screen_y = (int16_t)(game->player.y - town_camera_y(game));
    uint8_t animation_frame =
        (game->player.state == TOWN_PLAYER_WALKING) ? game->player.animation_frame : 0u;

    if (game->character == TOWN_CHARACTER_NEW) {
        town_draw_new_player(screen_x, screen_y, game->player.direction, animation_frame);
    } else {
        town_draw_soccer_player(screen_x, screen_y, game->player.direction, animation_frame,
                                game->player.state);
    }
}

static void town_draw_character_select(const town_map_game_t *game) {
    uint8_t selected_x = (game->selected_character == 0u) ? 20u : 84u;

    gfx_draw_text(32u, 3u, "CHOOSE PLAYER", 1u, true, false);
    gfx_draw_rect(selected_x, 17u, 25u, 28u, true);
    gfx_draw_rect((uint8_t)(selected_x + 2u), 19u, 21u, 24u, true);
    town_draw_soccer_player(24, 23, TOWN_DIR_DOWN, 0u, TOWN_PLAYER_STANDING);
    town_draw_new_player(88, 23, TOWN_DIR_DOWN, 0u);
    gfx_draw_text(21u, 49u, "SOCCER", 1u, true, false);
    gfx_draw_text(91u, 49u, "NEW", 1u, true, false);
    gfx_draw_text(45u, 58u, "MODE OK", 1u, true, false);
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void town_map_game_draw(const void *context) {
    const town_map_game_t *game = (const town_map_game_t *)context;

    if (game == NULL) {
        return;
    }
    gfx_clear();
    if (game->selecting_character) {
        town_draw_character_select(game);
        return;
    }
    if ((game->scene != TOWN_SCENE_TOWN) && (game->area_intro_ms > 0u)) {
        town_area_draw_intro(game->scene);
        return;
    }
    if (game->scene == TOWN_SCENE_FARM) {
        town_farm_draw(game);
        return;
    }
    if (game->scene == TOWN_SCENE_FISH_POND) {
        town_fish_pond_draw(game);
        return;
    }
    if (game->scene == TOWN_SCENE_TOWN) {
        town_draw_road(game);
        town_area_draw_town_signs(town_camera_x(game), town_camera_y(game));
    } else {
        town_area_draw(game, town_camera_x(game), town_camera_y(game));
    }
    town_draw_player(game);
    if (game->scene == TOWN_SCENE_TOWN) {
        town_farm_draw_coin_counter(game, 2u, 54u);
    }
}
