/**
 ******************************************************************************
 * @file    bike_racer_game.c
 * @brief   Bike racer game module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "bike_racer_game.h"

#include "graphics.h"
#include "audio.h"

#include <stddef.h>

#include "game_input.h"
#include "bike_racer_bitmap.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define BIKE_RACER_TRACK_LENGTH    (8900u)
#define BIKE_RACER_MAX_LAPS        (5u)
#define BIKE_RACER_MAX_SPEED       (250u)
#define BIKE_RACER_CRUISE_SPEED    (90u)
#define BIKE_RACER_MIN_CURVE_SPEED (60u)
#define BIKE_RACER_CURVE_DRAG_MIN  (2u)
#define BIKE_RACER_CURVE_DRAG_MAX  (2u)
#define BIKE_RACER_ROAD_WIDTH      (90)
#define BIKE_RACER_MIN_WIDTH       (10)
#define BIKE_RACER_PLAYER_Y        (44)
#define BIKE_RACER_ENGINE_SOUND_MS (90u)

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/

enum {
    BIKE_RACER_SEGMENT_NORMAL = 0u,
    BIKE_RACER_SEGMENT_LEFT = 1u << 0,
    BIKE_RACER_SEGMENT_RIGHT = 1u << 1,
    BIKE_RACER_SEGMENT_LEFT_WARNING = 1u << 5,
    BIKE_RACER_SEGMENT_RIGHT_WARNING = 1u << 6,
    BIKE_RACER_SEGMENT_FINISH = 1u << 7,
};

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef struct {
    uint8_t length_100;
    uint8_t type;
} bike_racer_segment_t;

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/

static const bike_racer_segment_t bike_racer_track[] = {
    {5u, BIKE_RACER_SEGMENT_NORMAL}, {7u, BIKE_RACER_SEGMENT_RIGHT_WARNING},
    {10u, BIKE_RACER_SEGMENT_RIGHT}, {12u, BIKE_RACER_SEGMENT_NORMAL},
    {2u, BIKE_RACER_SEGMENT_LEFT},   {2u, BIKE_RACER_SEGMENT_RIGHT},
    {5u, BIKE_RACER_SEGMENT_NORMAL}, {7u, BIKE_RACER_SEGMENT_LEFT_WARNING},
    {10u, BIKE_RACER_SEGMENT_LEFT},  {7u, BIKE_RACER_SEGMENT_RIGHT_WARNING},
    {20u, BIKE_RACER_SEGMENT_RIGHT}, {2u, BIKE_RACER_SEGMENT_FINISH},
};

static int16_t bike_racer_clamp(int16_t value, int16_t minimum, int16_t maximum) {
    if (value < minimum) {
        return minimum;
    }
    return (value > maximum) ? maximum : value;
}

static int16_t bike_racer_sin(int16_t degree) {
    degree %= 360;
    if (degree < 0) {
        degree += 360;
    }
    if (degree < 90) {
        return bike_racer_sine[degree];
    }
    if (degree < 180) {
        return bike_racer_sine[180 - degree];
    }
    if (degree < 270) {
        return -(int16_t)bike_racer_sine[degree - 180];
    }
    return -(int16_t)bike_racer_sine[360 - degree];
}

static uint8_t bike_racer_segment(uint32_t distance, uint16_t *position, uint16_t *length) {
    uint32_t segment_start = 0u;

    for (size_t index = 0u; index < sizeof(bike_racer_track) / sizeof(bike_racer_track[0]);
         index++) {
        uint32_t segment_size = (uint32_t)bike_racer_track[index].length_100 * 100u;
        if (distance < segment_start + segment_size) {
            *position = (uint16_t)(distance - segment_start);
            *length = (uint16_t)segment_size;
            return bike_racer_track[index].type;
        }
        segment_start += segment_size;
    }
    *position = 0u;
    *length = 1u;
    return BIKE_RACER_SEGMENT_NORMAL;
}

static int8_t bike_racer_target_curve(uint32_t distance) {
    uint16_t position;
    uint16_t length;
    uint8_t type = bike_racer_segment(distance, &position, &length);

    (void)position;
    (void)length;
    if ((type & BIKE_RACER_SEGMENT_LEFT) != 0u) {
        return -100;
    }
    if ((type & BIKE_RACER_SEGMENT_RIGHT) != 0u) {
        return 100;
    }
    return 0;
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void bike_racer_game_init(void *context, uint32_t seed) {
    bike_racer_game_t *game = (bike_racer_game_t *)context;

    (void)seed;
    if (game == NULL) {
        return;
    }
    game->elapsed_ms = 0u;
    game->state_ms = 0u;
    game->distance = 0u;
    game->update_accumulator_ms = 0u;
    game->player_x = 64;
    game->road_center = 64;
    game->curve = 0;
    game->drift_accumulator = 0;
    game->lean = 0;
    game->speed = 0u;
    game->lap = 0u;
    game->input_previous = 0u;
    game->return_to_menu = false;
    game->state = BIKE_RACER_READY;
    audio_player_init(&game->audio);
}

void bike_racer_game_deinit(void *context) {
    bike_racer_game_t *game = (bike_racer_game_t *)context;

    if (game != NULL) {
        audio_stop(&game->audio);
    }
}

void bike_racer_game_update(void *context, uint8_t input_state, uint32_t elapsed_ms) {
    bike_racer_game_t *game = (bike_racer_game_t *)context;
    uint8_t pressed;

    if (game == NULL) {
        return;
    }
    audio_update(&game->audio, elapsed_ms);
    pressed = (uint8_t)(input_state & ~game->input_previous);
    game->input_previous = input_state;
    game->state_ms += elapsed_ms;

    if (game->state == BIKE_RACER_READY) {
        if ((game->state_ms >= 3000u) || ((pressed & GAME_INPUT_MODE) != 0u)) {
            game->state = BIKE_RACER_RACING;
            game->state_ms = 0u;
            audio_play_tone(&game->audio, 1200u, 140u);
        }
        return;
    }
    if ((game->state == BIKE_RACER_FINISHED) || (game->state == BIKE_RACER_GAME_OVER)) {
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game->return_to_menu = true;
        } else if ((pressed & (GAME_INPUT_UP | GAME_INPUT_DOWN)) != 0u) {
            bike_racer_game_init(game, 0u);
        }
        return;
    }

    game->elapsed_ms += elapsed_ms;
    game->update_accumulator_ms += (uint16_t)elapsed_ms;
    if (game->update_accumulator_ms < 66u) {
        return;
    }
    game->update_accumulator_ms -= 66u;
    if ((input_state & GAME_INPUT_MODE) != 0u) {
        game->speed = (game->speed < BIKE_RACER_MAX_SPEED - 3u) ? (uint8_t)(game->speed + 3u)
                                                                : BIKE_RACER_MAX_SPEED;
        audio_play_tone(&game->audio, (uint16_t)(180u + game->speed * 4u),
                        BIKE_RACER_ENGINE_SOUND_MS);
    } else {
        /* Keep a gentle automatic acceleration like the original game. */
        game->speed =
            (game->speed < BIKE_RACER_CRUISE_SPEED) ? (uint8_t)(game->speed + 1u) : game->speed;
    }

    /* Gameplay-only mapping: DOWN steers left and UP steers right. */
    if ((input_state & GAME_INPUT_DOWN) != 0u) {
        game->player_x -= (int16_t)(2u + game->speed / 50u);
        game->lean = (game->lean > -6) ? (int8_t)(game->lean - 2) : -6;
    }
    if ((input_state & GAME_INPUT_UP) != 0u) {
        game->player_x += (int16_t)(2u + game->speed / 50u);
        game->lean = (game->lean < 6) ? (int8_t)(game->lean + 2) : 6;
    }
    if ((input_state & (GAME_INPUT_UP | GAME_INPUT_DOWN)) == 0u) {
        if (game->lean < 0) {
            game->lean++;
        } else if (game->lean > 0) {
            game->lean--;
        }
    }

    {
        int16_t target = bike_racer_target_curve(game->distance);
        int16_t step = (int16_t)(1u + game->speed / 40u);
        if (game->curve < target) {
            game->curve = bike_racer_clamp(game->curve + step, -100, target);
        } else if (game->curve > target) {
            game->curve = bike_racer_clamp(game->curve - step, target, 100);
        }
    }

    /* Steering through a bend costs 2 km/h per physics tick. MODE adds
       3 km/h before this, so throttle can compensate for cornering loss. */
    if ((game->curve != 0) && ((input_state & (GAME_INPUT_UP | GAME_INPUT_DOWN)) != 0u)) {
        uint16_t curve_amount =
            (game->curve < 0) ? (uint16_t)(-game->curve) : (uint16_t)game->curve;
        uint8_t drag =
            (uint8_t)(BIKE_RACER_CURVE_DRAG_MIN +
                      curve_amount * (BIKE_RACER_CURVE_DRAG_MAX - BIKE_RACER_CURVE_DRAG_MIN) /
                          100u);

        if (game->speed > BIKE_RACER_MIN_CURVE_SPEED) {
            uint8_t reduced_speed = (game->speed > drag) ? (uint8_t)(game->speed - drag) : 0u;

            game->speed = (reduced_speed < BIKE_RACER_MIN_CURVE_SPEED) ? BIKE_RACER_MIN_CURVE_SPEED
                                                                       : reduced_speed;
        }
    }
    /* Match the road center at the player's perspective row (y ~= 20). */
    game->road_center = (int16_t)(64 + game->curve / 21);
    /* Use curve directly and retain the fractional remainder. Short curves
       now push the bike immediately instead of disappearing in integer
       road-center rounding. Positive/right curves push toward the left. */
    game->drift_accumulator -= (int16_t)(game->curve * (1 + game->speed / 80));
    game->player_x += game->drift_accumulator / 63;
    game->drift_accumulator %= 63;
    game->player_x = bike_racer_clamp(game->player_x, 8, 119);

    /* At the player's row the road is about 57 px wide. Keep the bike's
       center inside it; leaving the asphalt ends the race immediately. */
    if ((game->player_x < game->road_center - 30) || (game->player_x > game->road_center + 30)) {
        game->speed = 0u;
        game->state = BIKE_RACER_GAME_OVER;
        game->state_ms = 0u;
        audio_play_tone(&game->audio, 180u, 350u);
        return;
    }

    game->distance += game->speed / 4u;
    if (game->distance >= BIKE_RACER_TRACK_LENGTH) {
        game->distance -= BIKE_RACER_TRACK_LENGTH;
        game->lap++;
        if (game->lap >= BIKE_RACER_MAX_LAPS) {
            game->state = BIKE_RACER_FINISHED;
            game->state_ms = 0u;
            audio_play_tone(&game->audio, 1800u, 400u);
        }
    }
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void bike_racer_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool on) {
    int16_t dx = (x1 > x0) ? x1 - x0 : x0 - x1;
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t dy = (y1 > y0) ? y0 - y1 : y1 - y0;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t error = dx + dy;

    while (true) {
        if ((x0 >= 0) && (x0 < 128) && (y0 >= 0) && (y0 < 64)) {
            gfx_draw_pixel((uint8_t)x0, (uint8_t)y0, on);
        }
        if ((x0 == x1) && (y0 == y1)) {
            break;
        }
        {
            int16_t twice = (int16_t)(2 * error);
            if (twice >= dy) {
                error += dy;
                x0 += sx;
            }
            if (twice <= dx) {
                error += dx;
                y0 += sy;
            }
        }
    }
}

static void bike_racer_draw_road(const bike_racer_game_t *game) {
    uint16_t segment_position;
    uint16_t segment_length;
    uint8_t segment_type = bike_racer_segment(game->distance, &segment_position, &segment_length);

    for (int16_t y = 24; y < 64; y++) {
        int16_t depth = y - 23;
        int16_t remaining = 63 - y;
        int16_t perspective_phase = (int16_t)((remaining * remaining * remaining) >> 6);
        int16_t center = 64 + game->curve / depth;
        int16_t width = BIKE_RACER_MIN_WIDTH + BIKE_RACER_ROAD_WIDTH * depth / 40;
        int16_t left = center - width / 2;
        int16_t right = center + width / 2;
        bool stripe = bike_racer_sin((int16_t)(game->distance + perspective_phase)) > 0;

        if (stripe) {
            if (left > 0) {
                gfx_fill_rect(0u, (uint8_t)y, (uint8_t)bike_racer_clamp(left, 0, 128), 1u, true);
            }
            if (right < 127) {
                int16_t grass_start = bike_racer_clamp(right, 0, 127);
                gfx_fill_rect((uint8_t)grass_start, (uint8_t)y, (uint8_t)(128 - grass_start), 1u,
                              true);
            }
        }
        {
            uint8_t border = (uint8_t)(1 + depth / 12);
            if (bike_racer_sin((int16_t)((game->distance + perspective_phase) * 4)) > 0) {
                gfx_fill_rect((uint8_t)bike_racer_clamp(left - border, 0, 127), (uint8_t)y, border,
                              1u, true);
                gfx_fill_rect((uint8_t)bike_racer_clamp(right, 0, 127), (uint8_t)y, border, 1u,
                              true);
            }
        }
    }

    if ((segment_type & BIKE_RACER_SEGMENT_FINISH) != 0u) {
        int16_t gate_y = 25 + (int16_t)((uint32_t)segment_position * 35u / segment_length);
        int16_t half = 8 + (gate_y - 24);
        int16_t center = 64 + game->curve / (gate_y - 22);
        bike_racer_draw_line(center - half, gate_y, center - half, 24, true);
        bike_racer_draw_line(center + half, gate_y, center + half, 24, true);
        gfx_fill_rect((uint8_t)bike_racer_clamp(center - half, 0, 127), 24u,
                      (uint8_t)bike_racer_clamp(half * 2, 1, 128), 3u, true);
    }
    if ((segment_type & (BIKE_RACER_SEGMENT_LEFT_WARNING | BIKE_RACER_SEGMENT_RIGHT_WARNING)) !=
        0u) {
        bool left_warning = (segment_type & BIKE_RACER_SEGMENT_LEFT_WARNING) != 0u;
        int16_t x = left_warning ? 104 : 24;
        int16_t point = left_warning ? x - 8 : x + 8;
        bike_racer_draw_line(point, 40, x, 34, true);
        bike_racer_draw_line(point, 40, x, 46, true);
        bike_racer_draw_line(x, 34, x, 46, true);
    }
}

static void bike_racer_draw_hud(const bike_racer_game_t *game) {
    gfx_fill_rect(0u, 0u, 128u, 9u, false);
    gfx_draw_text(1u, 1u, "Lap", 1u, true, false);
    gfx_draw_number(22u, 1u, (uint32_t)game->lap + 1u, 1u, true, false);
    gfx_draw_number(82u, 1u, game->speed, 1u, true, false);
    gfx_draw_text(103u, 1u, "km/h", 1u, true, false);
}

static void bike_racer_draw_player(const bike_racer_game_t *game) {
    int16_t base_x = game->player_x - 8;

    for (uint8_t row = 0u; row < BIKE_RACER_BIKE_HEIGHT; row++) {
        int16_t shift = game->lean * (7 - (int16_t)row) / 14;
        uint16_t mask =
            (uint16_t)bike_racer_bike_mask[row * 2u] << 8u | bike_racer_bike_mask[row * 2u + 1u];
        uint16_t bits = (uint16_t)bike_racer_bike[row * 2u] << 8u | bike_racer_bike[row * 2u + 1u];

        /* Clear the shaped silhouette first, never a rectangular backdrop. */
        for (uint8_t column = 0u; column < 16u; column++) {
            if ((mask & (uint16_t)(0x8000u >> column)) != 0u) {
                int16_t x = base_x + column + shift;
                if ((x >= 0) && (x < 128)) {
                    gfx_draw_pixel((uint8_t)x, (uint8_t)(BIKE_RACER_PLAYER_Y + row), false);
                }
            }
        }
        for (uint8_t column = 0u; column < 16u; column++) {
            if ((bits & (uint16_t)(0x8000u >> column)) != 0u) {
                int16_t x = base_x + column + shift;
                if ((x >= 0) && (x < 128)) {
                    gfx_draw_pixel((uint8_t)x, (uint8_t)(BIKE_RACER_PLAYER_Y + row), true);
                }
            }
        }
    }
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void bike_racer_game_draw(const void *context) {
    const bike_racer_game_t *game = (const bike_racer_game_t *)context;

    if (game == NULL) {
        return;
    }
    gfx_clear();
    if (game->state == BIKE_RACER_GAME_OVER) {
        gfx_draw_text(10u, 13u, "GAME OVER", 2u, true, false);
        gfx_draw_text(12u, 38u, "LEFT THE ROAD", 1u, true, false);
        gfx_draw_text(5u, 54u, "UP:RETRY MODE:MENU", 1u, true, false);
        return;
    }
    if (game->state == BIKE_RACER_FINISHED) {
        gfx_draw_text(38u, 13u, "FINISH", 2u, true, false);
        gfx_draw_text(22u, 38u, "TIME", 1u, true, false);
        gfx_draw_number(52u, 38u, game->elapsed_ms / 1000u, 1u, true, false);
        gfx_draw_text(76u, 38u, "SEC", 1u, true, false);
        gfx_draw_text(5u, 54u, "UP:RETRY MODE:MENU", 1u, true, false);
        return;
    }

    for (uint8_t star = 0u; star < 8u; star++) {
        uint8_t x = (uint8_t)((star * 37u + game->distance / 30u) & 127u);
        uint8_t y = (uint8_t)(11u + (star * 7u) % 11u);
        gfx_draw_pixel(x, y, true);
    }
    bike_racer_draw_road(game);
    bike_racer_draw_player(game);
    bike_racer_draw_hud(game);

    if (game->state == BIKE_RACER_READY) {
        uint8_t active_lights = (uint8_t)(game->state_ms / 500u);

        gfx_fill_rect(34u, 11u, 60u, 11u, true);
        for (uint8_t light = 0u; light < 5u; light++) {
            bool active = light < active_lights;
            gfx_fill_rect((uint8_t)(38u + light * 11u), 13u, 7u, 7u, !active);
            if (!active) {
                gfx_draw_rect((uint8_t)(38u + light * 11u), 13u, 7u, 7u, true);
            }
        }
    }
}
