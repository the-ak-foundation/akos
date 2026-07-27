#include "hollow_game.h"

#include <math.h>
#include <stddef.h>

#include "app.h"
#include "game_gfx.h"
#include "game_hal.h"
#include "hollow_assets.h"

#define HOLLOW_PI (3.14159265358979323846f)

static uint32_t hollow_random(hollow_game_t *game) {
    game->random_state = game->random_state * 1664525u + 1013904223u;
    return game->random_state;
}

static int16_t hollow_min(int16_t left, int16_t right) {
    return (left < right) ? left : right;
}

static int16_t hollow_max(int16_t left, int16_t right) {
    return (left > right) ? left : right;
}

static int16_t hollow_abs(int16_t value) {
    return (value < 0) ? -value : value;
}

static uint8_t hollow_mod_column(int16_t value) {
    while (value < 0) {
        value += HOLLOW_COLUMN_COUNT;
    }
    return (uint8_t)(value % HOLLOW_COLUMN_COUNT);
}

static bool hollow_is_space(hollow_game_t *game) {
    if (game->cave_hollow_count-- < 0) {
        game->cave_hollow_count =
            (int8_t)(((32768u + (hollow_random(game) & 0x7FFFu)) *
                       game->score) >> 22u);
        return true;
    }
    return false;
}

static void hollow_grow_column(hollow_game_t *game, uint8_t column,
                               bool is_space, uint8_t previous) {
    int16_t last_difference =
        game->columns[previous].bottom - game->columns[previous].top;
    int16_t current_difference = (int16_t)(hollow_random(game) % 3u);
    int16_t adjust;
    int16_t range;
    int16_t change;
    int16_t bottom;

    if (is_space) {
        current_difference = 8 - current_difference;
    }
    adjust = (game->columns[previous].bottom - 34) / 5;
    range = 17 - hollow_abs(current_difference - last_difference) -
            hollow_abs(adjust);
    if (range < 1) {
        range = 1;
    }
    change = (int16_t)(hollow_random(game) % (uint32_t)range) - 8;
    if (current_difference > last_difference) {
        change += current_difference - last_difference;
    }
    if (adjust < 0) {
        change -= adjust;
    }
    bottom = game->columns[previous].bottom + change;
    bottom = hollow_min(hollow_max(bottom, 16), 56);
    game->columns[column].top = (uint8_t)(bottom - current_difference);
    game->columns[column].bottom = (uint8_t)bottom;
}

static int16_t hollow_debris_y(const hollow_game_t *game, uint8_t position) {
    int16_t time = game->debris[position].time;
    return game->debris[position].base + (time * time >> 6);
}

void hollow_game_init(hollow_game_t *game) {
    if (game == NULL) {
        return;
    }

    game->random_state = 0x484F4C4Cu;
    game->score = 0u;
    game->counter = 120u;
    game->cave_phase = 0u;
    game->cave_loop_count = 0u;
    game->cave_gap = 0;
    game->cave_base_top = 0;
    game->cave_base_bottom = 0;
    game->cave_hollow_count = 0;
    game->cave_offset = 0u;
    game->cave_max_gap = 32u;
    game->player_x = 0u;
    game->player_column = 1u;
    game->player_move = 0u;
    game->player_jump = 0;
    game->player_direction = true;
    game->input_previous = 0u;
    game->events = HOLLOW_EVENT_START;
    game->starting = true;
    game->game_over = false;
    game->return_to_menu = false;

    for (uint8_t column = 0u; column < 9u; column++) {
        game->columns[column].top = 32u;
        game->columns[column].bottom = 40u;
    }
    for (uint8_t column = 9u; column < HOLLOW_COLUMN_COUNT; column++) {
        hollow_grow_column(game, column, hollow_is_space(game),
                           (uint8_t)(column - 1u));
    }
    for (uint16_t position = 0u; position < HOLLOW_CAVE_WIDTH; position++) {
        game->debris[position].time = 0u;
        game->debris[position].base = 0;
    }
}

void hollow_game_update(hollow_game_t *game, uint8_t input_state) {
    uint8_t pressed;

    if (game == NULL) {
        return;
    }

    pressed = (uint8_t)(input_state & ~game->input_previous);
    game->input_previous = input_state;

    if ((game->starting || game->game_over) && (game->counter > 0u)) {
        game->counter--;
        if ((game->counter == 0u) && game->starting) {
            game->starting = false;
        }
    }
    if (!game->starting) {
        game->cave_phase = (uint16_t)((game->cave_phase +
                                      (game->game_over ? 1u : 2u)) & 1023u);
    }

    game->cave_gap = (int16_t)((0.5f -
        cosf((float)game->cave_phase * HOLLOW_PI / 512.0f) * 0.5f) *
        game->cave_max_gap);
    game->cave_base_top = (int8_t)(-(game->cave_gap + 1) / 2);
    game->cave_base_bottom = (int8_t)(game->cave_gap / 2);

    if (game->game_over) {
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game->return_to_menu = true;
        } else if ((pressed & (GAME_INPUT_UP | GAME_INPUT_DOWN)) != 0u) {
            hollow_game_init(game);
        }
    } else if (game->player_move == 0u) {
        int8_t velocity = 0;

        if (game->starting) {
            velocity = 1;
        } else {
            /* Gameplay directions are intentionally swapped for this kit. */
            if ((input_state & GAME_INPUT_DOWN) != 0u) {
                velocity--;
            }
            if ((input_state & GAME_INPUT_UP) != 0u) {
                velocity++;
            }
        }
        if (velocity < 0) {
            game->player_direction = false;
            if (game->player_x == 0u) {
                velocity = 0;
            }
        } else if (velocity > 0) {
            game->player_direction = true;
        }
        if (velocity != 0) {
            uint8_t next_column = hollow_mod_column(
                (int16_t)game->player_column + velocity);
            int16_t next_gap = hollow_min(
                game->columns[game->player_column].bottom,
                game->columns[next_column].bottom) - hollow_max(
                game->columns[game->player_column].top,
                game->columns[next_column].top);

            if (next_gap + game->cave_gap >= 8) {
                game->player_jump = (int8_t)(
                    game->columns[game->player_column].bottom -
                    game->columns[next_column].bottom);
                game->player_column = next_column;
                game->player_move = 8u;
                game->events |= HOLLOW_EVENT_MOVE;
                if ((int16_t)game->player_x + velocity > 56) {
                    uint8_t grow_column = game->cave_offset / 8u;
                    hollow_grow_column(
                        game, grow_column, hollow_is_space(game),
                        hollow_mod_column((int16_t)grow_column - 1));
                    game->score++;
                }
            }
        }
    }

    if (game->player_move > 0u) {
        game->player_move--;
        if (game->player_direction) {
            game->player_x++;
        } else {
            game->player_x--;
        }
        if (game->player_x > 56u) {
            game->player_x--;
            game->cave_offset =
                (uint8_t)((game->cave_offset + 1u) % HOLLOW_CAVE_WIDTH);
        }
    }

    for (uint8_t position = 0u; position < HOLLOW_CAVE_WIDTH; position++) {
        if (game->debris[position].time > 0u) {
            int16_t bottom = game->columns[position / 8u].bottom +
                             game->cave_base_bottom;
            game->debris[position].time++;
            if (hollow_debris_y(game, position) > bottom) {
                game->debris[position].time = 0u;
            }
        }
    }
    if (!game->starting &&
        (hollow_random(game) % ((game->cave_phase >> 6u) + 1u) == 0u)) {
        uint8_t position = (uint8_t)(hollow_random(game) % HOLLOW_CAVE_WIDTH);
        if (game->debris[position].time == 0u) {
            game->debris[position].time = 1u;
            game->debris[position].base = (int8_t)(
                game->columns[position / 8u].top + game->cave_base_top);
        }
    }

    if (!game->starting && (game->cave_phase == 0u)) {
        game->cave_loop_count++;
        if (game->cave_max_gap < 255u) {
            game->cave_max_gap++;
        }
        if (game->columns[game->player_column].bottom -
                game->columns[game->player_column].top < 4) {
            game->game_over = true;
            game->counter = 480u;
            game->events |= HOLLOW_EVENT_GAME_OVER;
        }
    }
    if (!game->starting && !game->game_over &&
        (game->cave_phase == 976u)) {
        game->events |= HOLLOW_EVENT_CRUSH;
    }
}

uint8_t hollow_game_take_events(hollow_game_t *game) {
    uint8_t events;

    if (game == NULL) {
        return HOLLOW_EVENT_NONE;
    }
    events = game->events;
    game->events = HOLLOW_EVENT_NONE;
    return events;
}

static void hollow_draw_cave_top(uint8_t pattern, uint8_t x, int16_t height,
                                 int16_t edge, int16_t offset) {
    uint8_t shift;

    height -= offset;
    if (height < 0) {
        return;
    }
    edge = hollow_min(hollow_max(edge, 1), height);
    game_display_fill_rect(x, (uint8_t)(height - edge), 1u, (uint8_t)edge,
                           true);
    height -= edge;
    shift = (uint8_t)(-offset & 7);
    if (shift != 0u) {
        pattern = (uint8_t)((pattern << shift) | (pattern >> (8u - shift)));
    }
    for (int16_t page_y = 0; height > 0; page_y += 8, height -= 8) {
        uint8_t mask = (height < 8) ? (uint8_t)(0xFFu >> (8 - height)) : 0xFFu;
        uint8_t data = pattern & mask;

        for (uint8_t bit = 0u; bit < 8u; bit++) {
            if ((data & (1u << bit)) != 0u) {
                game_display_draw_pixel(x, (uint8_t)(page_y + bit), true);
            }
        }
    }
}

static void hollow_draw_cave_bottom(uint8_t pattern, uint8_t x,
                                    int16_t height, int16_t edge,
                                    int16_t offset) {
    uint8_t shift;

    height -= offset;
    if (height < 0) {
        return;
    }
    edge = hollow_min(hollow_max(edge, 1), height);
    game_display_fill_rect(x, (uint8_t)(GAME_DISPLAY_HEIGHT - height), 1u,
                           (uint8_t)edge, true);
    height -= edge;
    shift = (uint8_t)(-offset & 7);
    if (shift != 0u) {
        pattern = (uint8_t)((pattern >> shift) | (pattern << (8u - shift)));
    }
    for (int16_t page_y = 56; height > 0; page_y -= 8, height -= 8) {
        uint8_t mask = (height < 8) ? (uint8_t)(0xFFu << (8 - height)) : 0xFFu;
        uint8_t data = pattern & mask;

        for (uint8_t bit = 0u; bit < 8u; bit++) {
            if ((data & (1u << bit)) != 0u) {
                game_display_draw_pixel(x, (uint8_t)(page_y + bit), true);
            }
        }
    }
}

static void hollow_draw_cave(const hollow_game_t *game) {
    int16_t shake = (game->cave_phase > 976u) ?
                        (int16_t)((game->cave_phase & 3u) / 2u) : 0;
    int16_t offset_top = game->cave_base_top + shake;
    int16_t offset_bottom = game->cave_base_bottom + shake;

    for (uint16_t x = 0u; x < GAME_DISPLAY_WIDTH; x++) {
        uint8_t position = (uint8_t)((x + game->cave_offset + 8u) %
                                     HOLLOW_CAVE_WIDTH);
        uint8_t column = position / 8u;
        uint8_t odd = position & 7u;
        int16_t height;
        int16_t edge;
        uint8_t pattern = hollow_cave[(x + game->cave_offset) & 7u];

        height = game->columns[column].top;
        if (odd == 0u) {
            edge = height - game->columns[hollow_mod_column(column - 1)].top + 1;
        } else if (odd == 7u) {
            edge = height - game->columns[hollow_mod_column(column + 1)].top + 1;
        } else {
            edge = 1;
        }
        hollow_draw_cave_top(pattern, (uint8_t)x, height, edge,
                             -offset_top - HOLLOW_RENDER_Y_OFFSET);

        height = game->columns[column].bottom;
        if (odd == 0u) {
            edge = game->columns[hollow_mod_column(column - 1)].bottom - height + 1;
        } else if (odd == 7u) {
            edge = game->columns[hollow_mod_column(column + 1)].bottom - height + 1;
        } else {
            edge = 1;
        }
        hollow_draw_cave_bottom(pattern, (uint8_t)x,
                                GAME_DISPLAY_HEIGHT - height, edge,
                                offset_bottom + HOLLOW_RENDER_Y_OFFSET);
        if (game->debris[position].time > 0u) {
            int16_t debris_y = hollow_debris_y(game, position) + shake +
                               HOLLOW_RENDER_Y_OFFSET;
            if ((debris_y >= 0) &&
                (debris_y < (int16_t)GAME_DISPLAY_HEIGHT)) {
                game_display_draw_pixel((uint8_t)x, (uint8_t)debris_y, true);
            }
        }
    }
}

static void hollow_draw_score(uint16_t score) {
    const uint8_t x = 90u;
    const uint8_t y = 0u;
    const uint8_t width = 38u;
    const uint8_t height = 10u;
    uint8_t digits = 1u;
    uint16_t value = score;

    while (value >= 10u) {
        value /= 10u;
        digits++;
    }

    /* One compact, opaque score HUD is shared by every game state. */
    game_display_fill_rect(x, y, width, height, false);
    game_gfx_draw_rect(x, y, width, height, true);
    game_gfx_draw_number((uint8_t)(126u - digits * 6u), 1u, score, 1u,
                         true, false);
}

void hollow_game_draw(const hollow_game_t *game) {
    int16_t player_y;
    uint8_t animation;

    if (game == NULL) {
        return;
    }

    game_display_clear();
    hollow_draw_cave(game);

    player_y = game->columns[game->player_column].bottom +
               game->player_jump * game->player_move / 8 - 8;
    player_y = hollow_max(player_y + game->cave_base_bottom,
                          game->columns[game->player_column].top +
                          game->cave_base_top);
    if (game->game_over) {
        player_y = game->columns[game->player_column].top +
                   game->cave_base_bottom;
    }
    player_y += HOLLOW_RENDER_Y_OFFSET;
    player_y = hollow_min(hollow_max(player_y, 0), 56);
    animation = game->player_move / 2u;
    game_display_draw_bitmap(
        game->player_x, (uint8_t)player_y,
        &hollow_player[((game->player_direction ? 4u : 0u) + animation) * 8u],
        8u, 8u, true);

    if (game->starting) {
        game_display_fill_rect(32u, 20u, 64u, 18u, false);
        game_gfx_draw_text(34u, 21u, "READY", 2u, true, false);
    } else if (game->game_over) {
        game_display_fill_rect(8u, 20u, 112u, 18u, false);
        game_gfx_draw_text(10u, 21u, "GAME OVER", 2u, true, false);
        game_gfx_draw_text(5u, 42u, "UP:RETRY MODE:MENU", 1u,
                           true, false);
    }
    hollow_draw_score(game->score);
}
