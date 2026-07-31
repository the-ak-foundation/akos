#include "dino_game.h"

#include <stddef.h>
#include <string.h>

#include "app.h"
#include "dino_bitmap.h"
#include "game_gfx.h"
#include "game_hal.h"

#define DINO_UPDATE_MS             (16u)
#define DINO_PLAYER_X              (10)
#define DINO_GROUND_Y              (55)
#define DINO_STAND_WIDTH           (16u)
#define DINO_STAND_HEIGHT          (16u)
#define DINO_STAND_Y               (DINO_GROUND_Y - DINO_STAND_HEIGHT)
#define DINO_DUCK_WIDTH            (20u)
#define DINO_DUCK_HEIGHT           (12u)
#define DINO_DUCK_Y                (DINO_GROUND_Y - DINO_DUCK_HEIGHT)
#define DINO_JUMP_VELOCITY_Q4      (-44)
#define DINO_GRAVITY_Q4            (2)
#define DINO_INITIAL_SPEED_Q4      (16)
#define DINO_MAX_SPEED_Q4          (36)
#define DINO_SCORE_STEP            (75u)
#define DINO_SCORE_PERIOD_MS       (100u)
#define DINO_INITIAL_SPAWN_GAP_Q4  (60 * 16)
#define DINO_MIN_SPAWN_GAP_PIXELS  (72u)
#define DINO_SPAWN_VARIANCE_PIXELS (56u)
#define DINO_SCORE_X               (44u)
#define DINO_SCORE_Y               (0u)
#define DINO_GROUND_BITMAP_Y       (51)
#define DINO_GROUND_TILE_WIDTH     (64u)
#define DINO_GROUND_TILE_COUNT     (5u)
#define DINO_CACTUS_SINK_PIXELS    (4)

static uint32_t dino_random(dino_game_t *game) {
    uint32_t value = game->random_state;

    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    game->random_state = (value == 0u) ? 0x44494E4Fu : value;
    return game->random_state;
}

static int16_t dino_speed_q4(const dino_game_t *game) {
    uint32_t speed = DINO_INITIAL_SPEED_Q4 + game->score / DINO_SCORE_STEP;

    return (int16_t)((speed > DINO_MAX_SPEED_Q4) ?
                     DINO_MAX_SPEED_Q4 : speed);
}

static void dino_reset_round(dino_game_t *game, dino_state_t state) {
    memset(game->obstacles, 0, sizeof(game->obstacles));
    game->score = 0u;
    game->score_accumulator_ms = 0u;
    game->update_accumulator_ms = 0u;
    game->player_y_q4 = (int16_t)(DINO_STAND_Y * 16);
    game->player_velocity_q4 = 0;
    game->spawn_remaining_q4 = DINO_INITIAL_SPAWN_GAP_Q4;
    game->ground_phase_q4 = 0u;
    game->animation_frame = 0u;
    game->ducking = false;
    game->return_to_menu = false;
    game->state = state;
}

static bool dino_is_grounded(const dino_game_t *game) {
    return game->player_y_q4 >= (int16_t)(DINO_STAND_Y * 16);
}

static uint8_t dino_obstacle_width(uint8_t type) {
    switch (type) {
    case DINO_OBSTACLE_CACTUS_SMALL_BIG:
    case DINO_OBSTACLE_CACTUS_BIG_SMALL:
        return 14u;
    case DINO_OBSTACLE_CACTUS_BIG_BIG:
        return 18u;
    case DINO_OBSTACLE_CACTUS_THREE_SMALL:
        return 18u;
    default: return 12u;
    }
}

static uint8_t dino_obstacle_height(uint8_t type) {
    switch (type) {
    case DINO_OBSTACLE_CACTUS_SMALL_BIG:
    case DINO_OBSTACLE_CACTUS_BIG_SMALL:
    case DINO_OBSTACLE_CACTUS_BIG_BIG:
    case DINO_OBSTACLE_CACTUS_THREE_SMALL:
        return 18u;
    default: return 10u;
    }
}

static int16_t dino_obstacle_y(uint8_t type) {
    if (type == DINO_OBSTACLE_BIRD) {
        return 30;
    }
    return (int16_t)(DINO_GROUND_Y - dino_obstacle_height(type) +
                     DINO_CACTUS_SINK_PIXELS);
}

static bool dino_rectangles_overlap(int16_t left_a, int16_t top_a,
                                    int16_t width_a, int16_t height_a,
                                    int16_t left_b, int16_t top_b,
                                    int16_t width_b, int16_t height_b) {
    return (left_a < left_b + width_b) && (left_a + width_a > left_b) &&
           (top_a < top_b + height_b) && (top_a + height_a > top_b);
}

static bool dino_hits_obstacle(const dino_game_t *game,
                               const dino_obstacle_t *obstacle) {
    int16_t player_y = game->player_y_q4 / 16;
    int16_t obstacle_x = obstacle->x_q4 / 16;
    int16_t obstacle_y = dino_obstacle_y(obstacle->type);
    int16_t player_x;
    int16_t player_width;
    int16_t player_height;

    if (game->ducking && dino_is_grounded(game)) {
        player_x = DINO_PLAYER_X + 2;
        player_y = DINO_DUCK_Y + 4;
        player_width = 16;
        player_height = 7;
    } else {
        player_x = DINO_PLAYER_X + 2;
        player_y += 2;
        player_width = 12;
        player_height = 14;
    }

    if (obstacle->type == DINO_OBSTACLE_BIRD) {
        return dino_rectangles_overlap(
            player_x, player_y, player_width, player_height,
            obstacle_x + 1, obstacle_y + 2, 10, 6);
    }
    return dino_rectangles_overlap(
        player_x, player_y, player_width, player_height,
        obstacle_x + (dino_obstacle_width(obstacle->type) / 2) - 3,
        obstacle_y + 5, 6, 20);
}

static void dino_spawn_obstacle(dino_game_t *game) {
    for (uint8_t index = 0u; index < DINO_OBSTACLE_COUNT; index++) {
        dino_obstacle_t *obstacle = &game->obstacles[index];
        uint8_t type;

        if (obstacle->active) {
            continue;
        }
        type = (uint8_t)(dino_random(game) % DINO_OBSTACLE_TYPE_COUNT);
        if ((type == DINO_OBSTACLE_BIRD) && (game->score < 150u)) {
            type = (uint8_t)(dino_random(game) % 2u);
        }
        if ((type == DINO_OBSTACLE_CACTUS_BIG_BIG) &&
            (game->score < 100u)) {
            type = DINO_OBSTACLE_CACTUS_SMALL_BIG;
        }
        if ((type == DINO_OBSTACLE_CACTUS_THREE_SMALL) &&
            (game->score < 225u)) {
            type = DINO_OBSTACLE_CACTUS_BIG_SMALL;
        }
        obstacle->x_q4 = (int16_t)(GAME_DISPLAY_WIDTH * 16);
        obstacle->type = type;
        obstacle->active = true;
        game->spawn_remaining_q4 = (int16_t)(
            (DINO_MIN_SPAWN_GAP_PIXELS +
             dino_random(game) % DINO_SPAWN_VARIANCE_PIXELS) *
            16u);
        return;
    }
}

static void dino_update_clouds(dino_game_t *game, int16_t speed_q4) {
    for (uint8_t index = 0u; index < DINO_CLOUD_COUNT; index++) {
        dino_cloud_t *cloud = &game->clouds[index];

        cloud->x_q4 -= (int16_t)(speed_q4 / cloud->speed_divisor);
        if (cloud->x_q4 < -(int16_t)(20 * 16)) {
            cloud->x_q4 = (int16_t)(
                (GAME_DISPLAY_WIDTH + 16u + dino_random(game) % 32u) * 16u);
            cloud->y = (uint8_t)(12u + dino_random(game) % 17u);
        }
    }
}

static void dino_update_running(dino_game_t *game, uint8_t input_state,
                                uint8_t pressed) {
    int16_t speed_q4 = dino_speed_q4(game);

    if (((pressed & GAME_INPUT_MODE) != 0u) && dino_is_grounded(game)) {
        game->player_velocity_q4 = DINO_JUMP_VELOCITY_Q4;
        game->ducking = false;
    }
    game->ducking = ((input_state & GAME_INPUT_DOWN) != 0u) &&
                    dino_is_grounded(game);
    if (!dino_is_grounded(game) ||
        (game->player_velocity_q4 < 0)) {
        if ((input_state & GAME_INPUT_DOWN) != 0u) {
            game->player_velocity_q4 += DINO_GRAVITY_Q4;
        }
        game->player_velocity_q4 += DINO_GRAVITY_Q4;
        game->player_y_q4 += game->player_velocity_q4;
        if (game->player_y_q4 >= (int16_t)(DINO_STAND_Y * 16)) {
            game->player_y_q4 = (int16_t)(DINO_STAND_Y * 16);
            game->player_velocity_q4 = 0;
        }
    }

    game->ground_phase_q4 = (uint16_t)(
        (game->ground_phase_q4 + speed_q4) %
        (DINO_GROUND_TILE_WIDTH * DINO_GROUND_TILE_COUNT * 16u));
    game->animation_frame++;
    dino_update_clouds(game, speed_q4);

    game->spawn_remaining_q4 -= speed_q4;
    if (game->spawn_remaining_q4 <= 0) {
        dino_spawn_obstacle(game);
    }
    for (uint8_t index = 0u; index < DINO_OBSTACLE_COUNT; index++) {
        dino_obstacle_t *obstacle = &game->obstacles[index];

        if (!obstacle->active) {
            continue;
        }
        obstacle->x_q4 -= speed_q4;
        if (obstacle->x_q4 <
            -(int16_t)(dino_obstacle_width(obstacle->type) * 16u)) {
            obstacle->active = false;
            continue;
        }
        if (dino_hits_obstacle(game, obstacle)) {
            game->state = DINO_STATE_GAME_OVER;
            game->ducking = false;
            return;
        }
    }

    game->score_accumulator_ms += DINO_UPDATE_MS;
    while (game->score_accumulator_ms >= DINO_SCORE_PERIOD_MS) {
        game->score_accumulator_ms -= DINO_SCORE_PERIOD_MS;
        game->score++;
        if (game->score > game->high_score) {
            game->high_score = game->score;
        }
    }
}

void dino_game_init(void *context, uint32_t seed) {
    dino_game_t *game = (dino_game_t *)context;

    if (game == NULL) {
        return;
    }
    memset(game, 0, sizeof(*game));
    game->random_state = (seed == 0u) ? 0x44494E4Fu : seed;
    game->clouds[0] = (dino_cloud_t){20 * 16, 13u, 4u};
    game->clouds[1] = (dino_cloud_t){72 * 16, 24u, 5u};
    game->clouds[2] = (dino_cloud_t){116 * 16, 17u, 6u};
    dino_reset_round(game, DINO_STATE_READY);
}

void dino_game_deinit(void *context) {
    (void)context;
}

void dino_game_update(void *context, uint8_t input_state,
                      uint32_t elapsed_ms) {
    dino_game_t *game = (dino_game_t *)context;
    uint8_t pressed;

    if (game == NULL) {
        return;
    }
    pressed = (uint8_t)(input_state & ~game->input_previous);
    game->input_previous = input_state;

    if (game->state == DINO_STATE_READY) {
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            dino_reset_round(game, DINO_STATE_RUNNING);
            game->input_previous = input_state;
        }
        return;
    }
    if (game->state == DINO_STATE_GAME_OVER) {
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game->return_to_menu = true;
        } else if ((pressed & (GAME_INPUT_UP | GAME_INPUT_DOWN)) != 0u) {
            uint32_t high_score = game->high_score;

            dino_reset_round(game, DINO_STATE_RUNNING);
            game->high_score = high_score;
            game->input_previous = input_state;
        }
        return;
    }

    game->update_accumulator_ms += elapsed_ms;
    while (game->update_accumulator_ms >= DINO_UPDATE_MS) {
        game->update_accumulator_ms -= DINO_UPDATE_MS;
        dino_update_running(game, input_state, pressed);
        pressed = 0u;
        if (game->state != DINO_STATE_RUNNING) {
            break;
        }
    }
}

static void dino_score_digits(char *destination, uint32_t value) {
    value %= 100000u;
    for (int8_t index = 4; index >= 0; index--) {
        destination[index] = (char)('0' + value % 10u);
        value /= 10u;
    }
}

static void dino_draw_score(const dino_game_t *game) {
    char text[] = "HI 00000 00000";

    dino_score_digits(&text[3], game->high_score);
    dino_score_digits(&text[9], game->score);
    game_gfx_draw_text(DINO_SCORE_X, DINO_SCORE_Y, text, 1u, true, false);
}

static void dino_draw_bitmap_clipped(int16_t x, int16_t y,
                                     const uint8_t *bitmap,
                                     uint8_t width, uint8_t height) {
    for (uint8_t row = 0u; row < height; row++) {
        int16_t screen_y = y + row;

        if ((screen_y < 0) || (screen_y >= (int16_t)GAME_DISPLAY_HEIGHT)) {
            continue;
        }
        for (uint8_t column = 0u; column < width; column++) {
            int16_t screen_x = x + column;
            uint8_t data;

            if ((screen_x < 0) ||
                (screen_x >= (int16_t)GAME_DISPLAY_WIDTH)) {
                continue;
            }
            data = bitmap[(uint32_t)(row / 8u) * width + column];
            if ((data & (1u << (row & 7u))) != 0u) {
                game_display_draw_pixel((uint8_t)screen_x,
                                        (uint8_t)screen_y, true);
            }
        }
    }
}

static const uint8_t *dino_ground_tile(uint8_t index) {
    static const uint8_t *const tiles[DINO_GROUND_TILE_COUNT] = {
        dino_ground_1, dino_ground_2, dino_ground_3,
        dino_ground_4, dino_ground_5,
    };

    return tiles[index % DINO_GROUND_TILE_COUNT];
}

static void dino_draw_ground(const dino_game_t *game) {
    uint16_t phase = game->ground_phase_q4 / 16u;
    uint8_t tile_index =
        (uint8_t)((phase / DINO_GROUND_TILE_WIDTH) %
                  DINO_GROUND_TILE_COUNT);
    int16_t x = -(int16_t)(phase % DINO_GROUND_TILE_WIDTH);

    for (uint8_t visible = 0u; visible < 3u; visible++) {
        dino_draw_bitmap_clipped(
            x, DINO_GROUND_BITMAP_Y,
            dino_ground_tile((uint8_t)(tile_index + visible)),
            DINO_GROUND_TILE_WIDTH, 12u);
        x += DINO_GROUND_TILE_WIDTH;
    }
}

static void dino_draw_obstacle(const dino_obstacle_t *obstacle) {
    int16_t x = obstacle->x_q4 / 16;
    int16_t y;
    const uint8_t *bitmap;
    uint8_t width;
    uint8_t height;

    if (!obstacle->active || (x >= (int16_t)GAME_DISPLAY_WIDTH)) {
        return;
    }
    y = dino_obstacle_y(obstacle->type);
    switch (obstacle->type) {
    case DINO_OBSTACLE_CACTUS_SMALL_BIG:
        bitmap = dino_cactus_small_big;
        break;
    case DINO_OBSTACLE_CACTUS_BIG_SMALL:
        bitmap = dino_cactus_big_small;
        break;
    case DINO_OBSTACLE_CACTUS_BIG_BIG:
        bitmap = dino_cactus_big_big;
        break;
    case DINO_OBSTACLE_CACTUS_THREE_SMALL:
        bitmap = dino_cactus_three_small;
        break;
    default:
        bitmap = (((obstacle->x_q4 / 16) & 4) != 0) ?
                 dino_bird_1 : dino_bird_2;
        break;
    }
    width = dino_obstacle_width(obstacle->type);
    height = dino_obstacle_height(obstacle->type);
    dino_draw_bitmap_clipped(x, y, bitmap, width, height);
}

void dino_game_draw(const void *context) {
    const dino_game_t *game = (const dino_game_t *)context;
    int16_t player_y;

    if (game == NULL) {
        return;
    }
    game_display_clear();

    for (uint8_t index = 0u; index < DINO_CLOUD_COUNT; index++) {
        int16_t cloud_x = game->clouds[index].x_q4 / 16;

        if ((cloud_x >= 0) && (cloud_x <= 108)) {
            game_display_draw_bitmap((uint8_t)cloud_x, game->clouds[index].y,
                                     dino_cloud, 20u, 6u, true);
        }
    }
    dino_draw_score(game);
    dino_draw_ground(game);

    for (uint8_t index = 0u; index < DINO_OBSTACLE_COUNT; index++) {
        dino_draw_obstacle(&game->obstacles[index]);
    }

    player_y = game->player_y_q4 / 16;
    if (game->ducking && dino_is_grounded(game)) {
        dino_draw_bitmap_clipped(DINO_PLAYER_X, DINO_DUCK_Y, dino_duck,
                                 DINO_DUCK_WIDTH, DINO_DUCK_HEIGHT);
    } else {
        const uint8_t *sprite;

        if (game->state == DINO_STATE_GAME_OVER) {
            sprite = dino_crashed;
        } else if (game->state == DINO_STATE_READY) {
            sprite = dino_stand;
        } else {
            sprite = (((game->animation_frame / 6u) & 1u) != 0u) ?
                     dino_run_2 : dino_run_1;
        }

        dino_draw_bitmap_clipped(DINO_PLAYER_X, player_y, sprite,
                                 DINO_STAND_WIDTH, DINO_STAND_HEIGHT);
    }

    if (game->state == DINO_STATE_READY) {
        game_gfx_draw_text(37u, 25u, "MODE START", 1u, true, false);
    } else if (game->state == DINO_STATE_GAME_OVER) {
        game_display_fill_rect(33u, 18u, 62u, 31u, false);
        game_gfx_draw_text(37u, 20u, "GAME OVER", 1u, true, false);
        game_gfx_draw_text(40u, 31u, "UP RETRY", 1u, true, false);
        game_gfx_draw_text(37u, 41u, "MODE MENU", 1u, true, false);
    }
}
