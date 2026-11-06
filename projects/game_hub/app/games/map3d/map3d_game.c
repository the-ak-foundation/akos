/**
 ******************************************************************************
 * @file    map3d_game.c
 * @brief   Map3d game module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "map3d_game.h"

#include "graphics.h"

#include <stddef.h>
#include <string.h>

#include "game_input.h"

/*
 * Map coordinates, fixed-angle movement and sliding collision are adapted
 * from Arduboy3D by James Howard (MIT); see LICENSE in this directory.
 */

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
#define MAP3D_CELL_SIZE     (256)
#define MAP3D_PLAYER_RADIUS (40)
#define MAP3D_SIMULATION_MS (16u)
#define MAP3D_TURN_SPEED    (3)
#define MAP3D_MAP_HOLD_MS   (2000u)
#define MAP3D_MAP_BLINK_MS  (600u)
#define MAP3D_NEAR_PLANE    (65)
#define MAP3D_CLIP_PLANE    (32)
#define MAP3D_HORIZON       (32)
#define MAP3D_START_CELL_X  (MAP3D_WIDTH / 2u)
#define MAP3D_START_CELL_Y  (MAP3D_HEIGHT / 2u)

/* Quarter-wave Q10 lookup; angle units match Arduboy3D's 0..255 circle. */

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/
static const int16_t map3d_sine_quarter[65] = {
    0,   25,  50,  75,  100, 125,  150,  175,  200,  224,  249,  273,  297,  321, 345, 369, 392,
    415, 438, 460, 483, 505, 526,  548,  569,  590,  610,  630,  650,  669,  688, 706, 724, 742,
    759, 775, 792, 807, 822, 837,  851,  865,  878,  891,  903,  915,  926,  936, 946, 955, 964,
    972, 980, 987, 993, 999, 1004, 1009, 1013, 1016, 1019, 1021, 1023, 1024, 1024};

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static int16_t map3d_sine(uint8_t angle) {
    uint8_t quadrant = angle >> 6;
    uint8_t offset = angle & 63u;
    int16_t value;

    if ((quadrant & 1u) != 0u) {
        offset = (uint8_t)(64u - offset);
    }
    value = map3d_sine_quarter[offset];
    return (quadrant >= 2u) ? (int16_t)-value : value;
}

static int16_t map3d_cosine(uint8_t angle) {
    return map3d_sine((uint8_t)(angle + 64u));
}

static uint32_t map3d_random(map3d_game_t *game) {
    uint32_t value = game->random_state;

    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    game->random_state = value;
    return value;
}

static void map3d_set_wall(map3d_game_t *game, uint8_t x, uint8_t y, bool wall) {
    uint16_t index = (uint16_t)y * MAP3D_WIDTH + x;
    uint8_t mask = (uint8_t)(1u << (index & 7u));

    if (wall) {
        game->map[index >> 3] |= mask;
    } else {
        game->map[index >> 3] &= (uint8_t)~mask;
    }
}

static bool map3d_is_wall(const map3d_game_t *game, int16_t x, int16_t y) {
    uint16_t index;

    if ((x < 0) || (y < 0) || (x >= (int16_t)MAP3D_WIDTH) || (y >= (int16_t)MAP3D_HEIGHT)) {
        return true;
    }
    index = (uint16_t)y * MAP3D_WIDTH + (uint16_t)x;
    return (game->map[index >> 3] & (1u << (index & 7u))) != 0u;
}

static bool map3d_position_blocked(const map3d_game_t *game, int32_t x, int32_t y) {
    return map3d_is_wall(game, (int16_t)((x - MAP3D_PLAYER_RADIUS) >> 8),
                         (int16_t)((y - MAP3D_PLAYER_RADIUS) >> 8)) ||
           map3d_is_wall(game, (int16_t)((x + MAP3D_PLAYER_RADIUS) >> 8),
                         (int16_t)((y - MAP3D_PLAYER_RADIUS) >> 8)) ||
           map3d_is_wall(game, (int16_t)((x - MAP3D_PLAYER_RADIUS) >> 8),
                         (int16_t)((y + MAP3D_PLAYER_RADIUS) >> 8)) ||
           map3d_is_wall(game, (int16_t)((x + MAP3D_PLAYER_RADIUS) >> 8),
                         (int16_t)((y + MAP3D_PLAYER_RADIUS) >> 8));
}

static void map3d_split_room(map3d_game_t *game, uint8_t x, uint8_t y, uint8_t width,
                             uint8_t height) {
    const uint8_t minimum_room = 3u;
    const uint8_t maximum_room = 7u;

    if ((width <= maximum_room) && (height <= maximum_room)) {
        return;
    }
    if ((height > width) && (height > minimum_room * 2u + 1u)) {
        uint8_t range = (uint8_t)(height - minimum_room * 2u - 1u);
        uint8_t split = (uint8_t)(minimum_room + map3d_random(game) % (range + 1u));
        uint8_t wall_y = (uint8_t)(y + split);
        uint8_t door_x = (uint8_t)(x + 1u + map3d_random(game) % (width - 2u));

        for (uint8_t column = x; column < x + width; column++) {
            map3d_set_wall(game, column, wall_y, true);
        }
        map3d_set_wall(game, door_x, wall_y, false);
        map3d_split_room(game, x, y, width, split);
        map3d_split_room(game, x, (uint8_t)(wall_y + 1u), width, (uint8_t)(height - split - 1u));
    } else if (width > minimum_room * 2u + 1u) {
        uint8_t range = (uint8_t)(width - minimum_room * 2u - 1u);
        uint8_t split = (uint8_t)(minimum_room + map3d_random(game) % (range + 1u));
        uint8_t wall_x = (uint8_t)(x + split);
        uint8_t door_y = (uint8_t)(y + 1u + map3d_random(game) % (height - 2u));

        for (uint8_t row = y; row < y + height; row++) {
            map3d_set_wall(game, wall_x, row, true);
        }
        map3d_set_wall(game, wall_x, door_y, false);
        map3d_split_room(game, x, y, split, height);
        map3d_split_room(game, (uint8_t)(wall_x + 1u), y, (uint8_t)(width - split - 1u), height);
    }
}

static void map3d_generate(map3d_game_t *game) {
    memset(game->map, 0, sizeof(game->map));

    for (uint8_t y = 0u; y < MAP3D_HEIGHT; y++) {
        for (uint8_t x = 0u; x < MAP3D_WIDTH; x++) {
            if ((x == 0u) || (y == 0u) || (x == MAP3D_WIDTH - 1u) || (y == MAP3D_HEIGHT - 1u)) {
                map3d_set_wall(game, x, y, true);
            }
        }
    }
    map3d_split_room(game, 1u, 1u, MAP3D_WIDTH - 2u, MAP3D_HEIGHT - 2u);

    /* Keep the center spawn and its four exits clear of BSP divider walls. */
    map3d_set_wall(game, MAP3D_START_CELL_X, MAP3D_START_CELL_Y, false);
    map3d_set_wall(game, MAP3D_START_CELL_X - 1u, MAP3D_START_CELL_Y, false);
    map3d_set_wall(game, MAP3D_START_CELL_X + 1u, MAP3D_START_CELL_Y, false);
    map3d_set_wall(game, MAP3D_START_CELL_X, MAP3D_START_CELL_Y - 1u, false);
    map3d_set_wall(game, MAP3D_START_CELL_X, MAP3D_START_CELL_Y + 1u, false);
}

static void map3d_move(map3d_game_t *game, int16_t delta_x, int16_t delta_y) {
    int32_t next_x = (int32_t)game->player_x + delta_x;
    int32_t next_y = (int32_t)game->player_y + delta_y;

    /* Axis-separated collision reproduces the wall sliding of Arduboy3D. */
    if (!map3d_position_blocked(game, next_x, game->player_y)) {
        game->player_x = (uint16_t)next_x;
    }
    if (!map3d_position_blocked(game, game->player_x, next_y)) {
        game->player_y = (uint16_t)next_y;
    }
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void map3d_game_init(void *context, uint32_t seed) {
    map3d_game_t *game = (map3d_game_t *)context;

    memset(game, 0, sizeof(*game));
    game->random_state = (seed == 0u) ? 0x3D4D4150u : seed;
    map3d_generate(game);
    game->player_x = MAP3D_START_CELL_X * MAP3D_CELL_SIZE + MAP3D_CELL_SIZE / 2u;
    game->player_y = MAP3D_START_CELL_Y * MAP3D_CELL_SIZE + MAP3D_CELL_SIZE / 2u;
    game->angle = 0u;
}

void map3d_game_deinit(void *context) {
    (void)context;
}

void map3d_game_update(void *context, uint8_t input_state, uint32_t elapsed_ms) {
    map3d_game_t *game = (map3d_game_t *)context;
    uint8_t map_buttons = GAME_INPUT_DOWN | GAME_INPUT_MODE;

    game->map_blink_ms = (uint16_t)((game->map_blink_ms + elapsed_ms) % MAP3D_MAP_BLINK_MS);
    if (game->map_visible) {
        /*
         * Ignore the chord that opened the map until both buttons have been
         * released. The next fresh button press closes the overview without
         * leaking that input into movement or turning.
         */
        if (game->map_hold_latched) {
            if (input_state == 0u) {
                game->map_hold_latched = false;
                game->map_hold_ms = 0u;
            }
            return;
        }
        if (input_state != 0u) {
            game->map_visible = false;
            game->simulation_accumulator_ms = 0u;
        }
        return;
    }
    if ((input_state & map_buttons) == map_buttons) {
        if (!game->map_hold_latched) {
            uint32_t hold_ms = (uint32_t)game->map_hold_ms + elapsed_ms;

            game->map_hold_ms =
                (hold_ms >= MAP3D_MAP_HOLD_MS) ? MAP3D_MAP_HOLD_MS : (uint16_t)hold_ms;
            if (game->map_hold_ms >= MAP3D_MAP_HOLD_MS) {
                game->map_visible = !game->map_visible;
                game->map_hold_latched = true;
                game->velocity_x = 0;
                game->velocity_y = 0;
                game->angular_velocity = 0;
                game->simulation_accumulator_ms = 0u;
            }
        }
        return;
    }
    game->map_hold_ms = 0u;
    game->map_hold_latched = false;

    game->simulation_accumulator_ms = (uint16_t)(game->simulation_accumulator_ms + elapsed_ms);
    while (game->simulation_accumulator_ms >= MAP3D_SIMULATION_MS) {
        int8_t target_turn = 0;

        game->simulation_accumulator_ms -= MAP3D_SIMULATION_MS;
        if ((input_state & GAME_INPUT_DOWN) != 0u) {
            target_turn -= MAP3D_TURN_SPEED * 2;
        }
        if ((input_state & GAME_INPUT_MODE) != 0u) {
            target_turn += MAP3D_TURN_SPEED * 2;
        }
        if (game->angular_velocity < target_turn) {
            game->angular_velocity++;
        } else if (game->angular_velocity > target_turn) {
            game->angular_velocity--;
        }
        game->angle = (uint8_t)(game->angle + (game->angular_velocity >> 1));

        if ((input_state & GAME_INPUT_UP) != 0u) {
            game->velocity_x += map3d_cosine(game->angle) / 96;
            game->velocity_y += map3d_sine(game->angle) / 96;
        }
        map3d_move(game, game->velocity_x / 4, game->velocity_y / 4);
        game->velocity_x = (int16_t)(game->velocity_x * 7 / 8);
        game->velocity_y = (int16_t)(game->velocity_y * 7 / 8);
    }
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void map3d_draw_background(void) {
    /* Same sparse ceiling / dense floor dither used by Arduboy3D. */
    for (uint8_t y = 0u; y < GFX_HEIGHT; y++) {
        for (uint8_t x = 0u; x < GFX_WIDTH; x++) {
            bool on;

            if (y < MAP3D_HORIZON) {
                on = (((x + (y & 1u) * 2u) & 3u) == 0u) && ((y & 1u) == 0u);
            } else {
                on = ((y & 1u) != 0u) || (((x + (y & 1u) * 2u) & 3u) == 0u);
            }
            if (on) {
                gfx_draw_pixel(x, y, true);
            }
        }
    }
}

static void map3d_transform(const map3d_game_t *game, int16_t world_x, int16_t world_y,
                            int16_t *view_x, int16_t *view_z) {
    int32_t relative_x = world_x - game->player_x;
    int32_t relative_y = world_y - game->player_y;
    int16_t rotation_cos = map3d_cosine(game->angle);
    int16_t rotation_sin = (int16_t)-map3d_sine(game->angle);

    *view_z =
        (int16_t)(((int32_t)rotation_cos * relative_x - (int32_t)rotation_sin * relative_y) >> 10);
    *view_x =
        (int16_t)(((int32_t)rotation_sin * relative_x + (int32_t)rotation_cos * relative_y) >> 10);
}

static void map3d_draw_wall(const map3d_game_t *game, int16_t world_x1, int16_t world_y1,
                            int16_t world_x2, int16_t world_y2, bool shaded,
                            uint8_t depth[GFX_WIDTH]) {
    int16_t view_x1;
    int16_t view_z1;
    int16_t view_x2;
    int16_t view_z2;
    int16_t screen_x1;
    int16_t screen_x2;
    int16_t half_height1;
    int16_t half_height2;
    int16_t texture1 = 0;
    int16_t texture2 = 16;

    map3d_transform(game, world_x1, world_y1, &view_x1, &view_z1);
    map3d_transform(game, world_x2, world_y2, &view_x2, &view_z2);
    if ((view_z1 < MAP3D_CLIP_PLANE) && (view_z2 < MAP3D_CLIP_PLANE)) {
        return;
    }
    if (view_z1 < MAP3D_CLIP_PLANE) {
        texture1 += (MAP3D_CLIP_PLANE - view_z1) * (texture2 - texture1) / (view_z2 - view_z1);
        view_x1 += (MAP3D_CLIP_PLANE - view_z1) * (view_x2 - view_x1) / (view_z2 - view_z1);
        view_z1 = MAP3D_CLIP_PLANE;
    } else if (view_z2 < MAP3D_CLIP_PLANE) {
        texture2 += (MAP3D_CLIP_PLANE - view_z2) * (texture1 - texture2) / (view_z1 - view_z2);
        view_x2 += (MAP3D_CLIP_PLANE - view_z2) * (view_x1 - view_x2) / (view_z1 - view_z2);
        view_z2 = MAP3D_CLIP_PLANE;
    }

    screen_x1 = (int16_t)(GFX_WIDTH / 2u + (int32_t)view_x1 * MAP3D_NEAR_PLANE / view_z1);
    screen_x2 = (int16_t)(GFX_WIDTH / 2u + (int32_t)view_x2 * MAP3D_NEAR_PLANE / view_z2 - 1);
    if ((screen_x1 >= screen_x2) || (screen_x2 < 0) || (screen_x1 >= (int16_t)GFX_WIDTH)) {
        return;
    }
    half_height1 = (MAP3D_CELL_SIZE / 2 * MAP3D_NEAR_PLANE) / view_z1;
    half_height2 = (MAP3D_CELL_SIZE / 2 * MAP3D_NEAR_PLANE) / view_z2;

    {
        int16_t clipped_x1 = (screen_x1 < 0) ? 0 : screen_x1;
        int16_t clipped_x2 = (screen_x2 >= (int16_t)GFX_WIDTH) ? (int16_t)GFX_WIDTH - 1 : screen_x2;
        int16_t width = screen_x2 - screen_x1;

        for (int16_t x = clipped_x1; x <= clipped_x2; x++) {
            int16_t offset = x - screen_x1;
            int16_t half_height =
                half_height1 + (int32_t)offset * (half_height2 - half_height1) / width;
            int16_t texture_u = texture1 + (int32_t)offset * (texture2 - texture1) / width;
            int16_t top;
            int16_t bottom;

            if (half_height > 63) {
                half_height = 63;
            }
            if (depth[x] >= (uint8_t)half_height) {
                continue;
            }
            depth[x] = (uint8_t)half_height;
            top = MAP3D_HORIZON - half_height;
            bottom = MAP3D_HORIZON + half_height;
            if (top < 0) {
                top = 0;
            }
            if (bottom >= (int16_t)GFX_HEIGHT) {
                bottom = (int16_t)GFX_HEIGHT - 1;
            }
            for (int16_t y = top; y <= bottom; y++) {
                uint8_t texture_v = (uint8_t)((int32_t)(y - (MAP3D_HORIZON - half_height)) * 16 /
                                              (half_height * 2));
                uint8_t texture_column = (uint8_t)((texture_u < 0) ? 0 : texture_u);
                bool vector_line =
                    (texture_v == 5u) || (texture_v == 10u) ||
                    ((texture_column == 5u) && (texture_v < 5u)) ||
                    ((texture_column == 10u) && (texture_v >= 5u) && (texture_v < 10u)) ||
                    ((texture_column == 8u) && (texture_v >= 10u));
                bool on = !vector_line;

                if (shaded && (((x + y) & 1) == 0)) {
                    on = false;
                }
                gfx_draw_pixel((uint8_t)x, (uint8_t)y, on);
            }
        }
    }
}

static void map3d_draw_cell(const map3d_game_t *game, uint8_t x, uint8_t y,
                            uint8_t depth[GFX_WIDTH]) {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;

    if (!map3d_is_wall(game, x, y)) {
        return;
    }
    x1 = (int16_t)x * MAP3D_CELL_SIZE;
    y1 = (int16_t)y * MAP3D_CELL_SIZE;
    x2 = x1 + MAP3D_CELL_SIZE;
    y2 = y1 + MAP3D_CELL_SIZE;

    /* Endpoint order and camera-facing checks follow DrawCell in Arduboy3D. */
    if (!map3d_is_wall(game, (int16_t)x - 1, y) && game->player_x < (uint16_t)x1) {
        map3d_draw_wall(game, x1, y1, x1, y2, true, depth);
    }
    if (!map3d_is_wall(game, x, (int16_t)y + 1) && game->player_y > (uint16_t)y2) {
        map3d_draw_wall(game, x1, y2, x2, y2, false, depth);
    }
    if (!map3d_is_wall(game, (int16_t)x + 1, y) && game->player_x > (uint16_t)x2) {
        map3d_draw_wall(game, x2, y2, x2, y1, true, depth);
    }
    if (!map3d_is_wall(game, x, (int16_t)y - 1) && game->player_y < (uint16_t)y1) {
        map3d_draw_wall(game, x2, y1, x1, y1, false, depth);
    }
}

static void map3d_draw_player_marker(uint8_t center_x, uint8_t center_y) {
    /* Five-pixel diamond, centered exactly in the current 3x3 map cell. */
    gfx_draw_pixel(center_x, center_y, true);
    gfx_draw_pixel(center_x, (uint8_t)(center_y - 1u), true);
    gfx_draw_pixel(center_x, (uint8_t)(center_y + 1u), true);
    gfx_draw_pixel((uint8_t)(center_x - 1u), center_y, true);
    gfx_draw_pixel((uint8_t)(center_x + 1u), center_y, true);
}

static void map3d_draw_overview(const map3d_game_t *game) {
    const uint8_t scale = 3u;
    const uint8_t origin_x = (GFX_WIDTH - MAP3D_WIDTH * scale) / 2u;
    const uint8_t origin_y = (GFX_HEIGHT - MAP3D_HEIGHT * scale) / 2u;

    gfx_clear();
    for (uint8_t y = 0u; y < MAP3D_HEIGHT; y++) {
        for (uint8_t x = 0u; x < MAP3D_WIDTH; x++) {
            if (map3d_is_wall(game, x, y)) {
                gfx_fill_rect((uint8_t)(origin_x + x * scale), (uint8_t)(origin_y + y * scale),
                              scale, scale, true);
            }
        }
    }
    if (game->map_blink_ms < MAP3D_MAP_BLINK_MS / 2u) {
        uint8_t player_x = (uint8_t)(game->player_x >> 8);
        uint8_t player_y = (uint8_t)(game->player_y >> 8);
        uint8_t center_x = (uint8_t)(origin_x + player_x * scale + scale / 2u);
        uint8_t center_y = (uint8_t)(origin_y + player_y * scale + scale / 2u);

        /* Clear one cell so the directional marker remains readable. */
        gfx_fill_rect((uint8_t)(origin_x + player_x * scale),
                      (uint8_t)(origin_y + player_y * scale), scale, scale, false);
        map3d_draw_player_marker(center_x, center_y);
    }
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void map3d_game_draw(const void *context) {
    const map3d_game_t *game = (const map3d_game_t *)context;
    uint8_t depth[GFX_WIDTH] = {0};

    if (game->map_visible) {
        map3d_draw_overview(game);
        return;
    }
    gfx_clear();
    map3d_draw_background();

    /*
     * The original uses direction-dependent front-to-back traversal. The
     * per-column depth buffer makes the result order-independent here while
     * retaining the same projected wall-segment algorithm.
     */
    for (uint8_t x = 0u; x < MAP3D_WIDTH; x++) {
        for (uint8_t y = 0u; y < MAP3D_HEIGHT; y++) {
            map3d_draw_cell(game, x, y, depth);
        }
    }
}
