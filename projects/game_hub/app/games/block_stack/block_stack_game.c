#include "block_stack_game.h"

#include <stddef.h>
#include <string.h>

#include "app.h"
#include "game_gfx.h"
#include "game_hal.h"

#define BLOCK_LEFT_WALL_X         (0u)
#define BLOCK_LEFT_WALL_WIDTH     (17u)
#define BLOCK_BOARD_X             (17u)
#define BLOCK_BOARD_Y             (1u)
#define BLOCK_CELL_SIZE           (4u)
#define BLOCK_GRID_X              (19u)
#define BLOCK_GRID_Y              (2u)
#define BLOCK_BOARD_PIXEL_WIDTH   (BLOCK_BOARD_WIDTH * BLOCK_CELL_SIZE)
#define BLOCK_BOARD_PIXEL_HEIGHT  (BLOCK_BOARD_HEIGHT * BLOCK_CELL_SIZE)
#define BLOCK_BOARD_FRAME_WIDTH   (BLOCK_BOARD_PIXEL_WIDTH + 4u)
#define BLOCK_FRAME_HEIGHT        (BLOCK_BOARD_PIXEL_HEIGHT + 2u)
#define BLOCK_PANEL_X             (60u)
#define BLOCK_PANEL_WIDTH         (51u)
#define BLOCK_PANEL_CENTER_X      (85u)
#define BLOCK_RIGHT_WALL_X        (111u)
#define BLOCK_RIGHT_WALL_WIDTH    (17u)
#define BLOCK_WALL_ROW_HEIGHT     (8u)
#define BLOCK_PREVIEW_X           (67u)
#define BLOCK_PREVIEW_Y           (5u)
#define BLOCK_PREVIEW_WIDTH       (37u)
#define BLOCK_PREVIEW_HEIGHT      (18u)
#define BLOCK_PREVIEW_CELL_SIZE   (3u)
#define BLOCK_SCORE_LABEL_Y       (28u)
#define BLOCK_SCORE_VALUE_Y       (38u)
#define BLOCK_LEVEL_ROW_Y         (52u)
#define BLOCK_FONT_WIDTH          (6u)
#define BLOCK_LEVEL_ROW_WIDTH     (5u * BLOCK_FONT_WIDTH)
#define BLOCK_INITIAL_FALL_MS     (500u)
#define BLOCK_MINIMUM_FALL_MS     (100u)
#define BLOCK_LEVEL_STEP_MS       (50u)
#define BLOCK_LEVEL_SCORE         (500u)
#define BLOCK_MAXIMUM_LEVEL       (9u)
#define BLOCK_SPAWN_X             (3)
#define BLOCK_SPAWN_Y             (0)

/*
 * Each rotation contains four 4-bit rows. Bit zero is the left-most cell.
 * Keeping all rotations in Flash avoids rotation buffers and signed matrix
 * arithmetic in the update path.
 */
static const uint8_t block_shapes[BLOCK_PIECE_COUNT][4][4] = {
    {
        {0x0u, 0xFu, 0x0u, 0x0u},
        {0x2u, 0x2u, 0x2u, 0x2u},
        {0x0u, 0x0u, 0xFu, 0x0u},
        {0x4u, 0x4u, 0x4u, 0x4u},
    },
    {
        {0x6u, 0x6u, 0x0u, 0x0u},
        {0x6u, 0x6u, 0x0u, 0x0u},
        {0x6u, 0x6u, 0x0u, 0x0u},
        {0x6u, 0x6u, 0x0u, 0x0u},
    },
    {
        {0x2u, 0x7u, 0x0u, 0x0u},
        {0x2u, 0x3u, 0x2u, 0x0u},
        {0x0u, 0x7u, 0x2u, 0x0u},
        {0x2u, 0x6u, 0x2u, 0x0u},
    },
    {
        {0x3u, 0x6u, 0x0u, 0x0u},
        {0x2u, 0x3u, 0x1u, 0x0u},
        {0x3u, 0x6u, 0x0u, 0x0u},
        {0x2u, 0x3u, 0x1u, 0x0u},
    },
    {
        {0x6u, 0x3u, 0x0u, 0x0u},
        {0x1u, 0x3u, 0x2u, 0x0u},
        {0x6u, 0x3u, 0x0u, 0x0u},
        {0x1u, 0x3u, 0x2u, 0x0u},
    },
    {
        {0x1u, 0x7u, 0x0u, 0x0u},
        {0x3u, 0x2u, 0x2u, 0x0u},
        {0x0u, 0x7u, 0x4u, 0x0u},
        {0x2u, 0x2u, 0x6u, 0x0u},
    },
    {
        {0x4u, 0x7u, 0x0u, 0x0u},
        {0x2u, 0x2u, 0x3u, 0x0u},
        {0x0u, 0x7u, 0x1u, 0x0u},
        {0x6u, 0x2u, 0x2u, 0x0u},
    },
};

static uint32_t block_random(block_stack_game_t *game) {
    uint32_t value = game->random_state;

    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    game->random_state = (value == 0u) ? 0x424C4F43u : value;
    return game->random_state;
}

static uint8_t block_random_piece(block_stack_game_t *game) {
    return (uint8_t)(block_random(game) % BLOCK_PIECE_COUNT);
}

static bool block_shape_cell(uint8_t type, uint8_t rotation,
                             uint8_t x, uint8_t y) {
    return (block_shapes[type][rotation & 3u][y] & (1u << x)) != 0u;
}

bool block_stack_piece_fits(const block_stack_game_t *game,
                            const block_piece_t *piece) {
    if ((game == NULL) || (piece == NULL) ||
        (piece->type >= BLOCK_PIECE_COUNT)) {
        return false;
    }
    for (uint8_t y = 0u; y < 4u; y++) {
        for (uint8_t x = 0u; x < 4u; x++) {
            int8_t board_x;
            int8_t board_y;

            if (!block_shape_cell(piece->type, piece->rotation, x, y)) {
                continue;
            }
            board_x = (int8_t)(piece->x + x);
            board_y = (int8_t)(piece->y + y);
            if ((board_x < 0) || (board_x >= (int8_t)BLOCK_BOARD_WIDTH) ||
                (board_y < 0) || (board_y >= (int8_t)BLOCK_BOARD_HEIGHT) ||
                ((game->board[(uint8_t)board_y] &
                  (1u << (uint8_t)board_x)) != 0u)) {
                return false;
            }
        }
    }
    return true;
}

static void block_stack_spawn_piece(block_stack_game_t *game) {
    game->current_piece.type = game->next_piece;
    game->current_piece.rotation = 0u;
    game->current_piece.x = BLOCK_SPAWN_X;
    game->current_piece.y = BLOCK_SPAWN_Y;
    game->next_piece = block_random_piece(game);
    game->fall_accumulator_ms = 0u;
    if (!block_stack_piece_fits(game, &game->current_piece)) {
        game->game_over = true;
    }
}

static void block_stack_update_level(block_stack_game_t *game) {
    uint32_t calculated_level = game->score / BLOCK_LEVEL_SCORE + 1u;

    if (calculated_level > BLOCK_MAXIMUM_LEVEL) {
        calculated_level = BLOCK_MAXIMUM_LEVEL;
    }
    game->level = (uint8_t)calculated_level;
    game->fall_interval_ms = (uint16_t)(
        BLOCK_INITIAL_FALL_MS -
        (uint16_t)(game->level - 1u) * BLOCK_LEVEL_STEP_MS);
    if (game->fall_interval_ms < BLOCK_MINIMUM_FALL_MS) {
        game->fall_interval_ms = BLOCK_MINIMUM_FALL_MS;
    }
}

uint8_t block_stack_clear_lines(block_stack_game_t *game) {
    const uint16_t full_row = (1u << BLOCK_BOARD_WIDTH) - 1u;
    uint8_t cleared = 0u;

    if (game == NULL) {
        return 0u;
    }
    for (int8_t row = (int8_t)BLOCK_BOARD_HEIGHT - 1; row >= 0; row--) {
        if (game->board[(uint8_t)row] != full_row) {
            continue;
        }
        for (int8_t move = row; move > 0; move--) {
            game->board[(uint8_t)move] =
                game->board[(uint8_t)(move - 1)];
        }
        game->board[0] = 0u;
        cleared++;
        row++;
    }

    switch (cleared) {
    case 1u: game->score += 100u; break;
    case 2u: game->score += 300u; break;
    case 3u: game->score += 500u; break;
    case 4u: game->score += 800u; break;
    default: break;
    }
    game->lines_cleared += cleared;
    block_stack_update_level(game);
    return cleared;
}

static void block_stack_lock_piece(block_stack_game_t *game) {
    for (uint8_t y = 0u; y < 4u; y++) {
        for (uint8_t x = 0u; x < 4u; x++) {
            if (block_shape_cell(game->current_piece.type,
                                 game->current_piece.rotation, x, y)) {
                uint8_t board_x =
                    (uint8_t)(game->current_piece.x + x);
                uint8_t board_y =
                    (uint8_t)(game->current_piece.y + y);

                game->board[board_y] |= (uint16_t)(1u << board_x);
            }
        }
    }
    block_stack_clear_lines(game);
    block_stack_spawn_piece(game);
}

static void block_stack_reset(block_stack_game_t *game, uint32_t seed) {
    memset(game, 0, sizeof(*game));
    game->random_state = (seed == 0u) ? 0x424C4F43u : seed;
    block_stack_update_level(game);
    game->next_piece = block_random_piece(game);
    block_stack_spawn_piece(game);
}

void block_stack_game_init(void *context, uint32_t seed) {
    if (context != NULL) {
        block_stack_reset((block_stack_game_t *)context, seed);
    }
}

void block_stack_game_deinit(void *context) {
    (void)context;
}

void block_stack_game_update(void *context, uint8_t input_state,
                             uint32_t elapsed_ms) {
    block_stack_game_t *game = (block_stack_game_t *)context;
    uint8_t pressed;

    if (game == NULL) {
        return;
    }
    pressed = (uint8_t)(input_state & ~game->input_previous);
    game->input_previous = input_state;
    if (game->game_over) {
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            block_stack_reset(game, game->random_state ^ game->score);
        } else if ((pressed & (GAME_INPUT_UP | GAME_INPUT_DOWN)) != 0u) {
            game->return_to_menu = true;
        }
        return;
    }

    if ((pressed & GAME_INPUT_DOWN) != 0u) {
        block_piece_t moved = game->current_piece;

        moved.x--;
        if (block_stack_piece_fits(game, &moved)) {
            game->current_piece = moved;
        }
    }
    if ((pressed & GAME_INPUT_UP) != 0u) {
        block_piece_t moved = game->current_piece;

        moved.x++;
        if (block_stack_piece_fits(game, &moved)) {
            game->current_piece = moved;
        }
    }
    if ((pressed & GAME_INPUT_MODE) != 0u) {
        block_piece_t rotated = game->current_piece;

        rotated.rotation = (uint8_t)((rotated.rotation + 1u) & 3u);
        if (block_stack_piece_fits(game, &rotated)) {
            game->current_piece = rotated;
        }
    }

    game->fall_accumulator_ms += elapsed_ms;
    while (game->fall_accumulator_ms >= game->fall_interval_ms) {
        block_piece_t fallen = game->current_piece;

        game->fall_accumulator_ms -= game->fall_interval_ms;
        fallen.y++;
        if (block_stack_piece_fits(game, &fallen)) {
            game->current_piece = fallen;
        } else {
            block_stack_lock_piece(game);
            break;
        }
    }
}

static void block_stack_draw_cell(uint8_t x, uint8_t y, uint8_t size) {
    game_display_fill_rect(x, y, (uint8_t)(size - 1u),
                           (uint8_t)(size - 1u), true);
}

static void block_stack_draw_piece(const block_piece_t *piece,
                                   uint8_t origin_x, uint8_t origin_y,
                                   uint8_t cell_size) {
    for (uint8_t y = 0u; y < 4u; y++) {
        for (uint8_t x = 0u; x < 4u; x++) {
            if (block_shape_cell(piece->type, piece->rotation, x, y)) {
                block_stack_draw_cell(
                    (uint8_t)(origin_x + x * cell_size),
                    (uint8_t)(origin_y + y * cell_size), cell_size);
            }
        }
    }
}

static void block_stack_draw_wall(uint8_t origin_x, uint8_t width) {
    for (uint8_t y = 0u; y < GAME_DISPLAY_HEIGHT;
         y = (uint8_t)(y + BLOCK_WALL_ROW_HEIGHT)) {
        uint8_t joint_x = (uint8_t)(
            origin_x + ((((y / BLOCK_WALL_ROW_HEIGHT) & 1u) != 0u)
                            ? 5u
                            : 10u));

        game_display_fill_rect(origin_x, y, width, 1u, true);
        game_display_fill_rect(joint_x, y, 1u, BLOCK_WALL_ROW_HEIGHT, true);
    }
    game_display_fill_rect(origin_x, 63u, width, 1u, true);
}

static uint8_t block_stack_decimal_width(uint32_t value) {
    uint8_t digits = 1u;

    while (value >= 10u) {
        value /= 10u;
        digits++;
    }
    return (uint8_t)(digits * BLOCK_FONT_WIDTH);
}

static uint8_t block_stack_center_x(uint8_t pixel_width) {
    return (uint8_t)(BLOCK_PANEL_CENTER_X - pixel_width / 2u);
}

static void block_stack_draw_preview(uint8_t piece_type) {
    uint8_t minimum_x = 4u;
    uint8_t maximum_x = 0u;
    uint8_t minimum_y = 4u;
    uint8_t maximum_y = 0u;
    block_piece_t preview = {
        .x = 0,
        .y = 0,
        .type = piece_type,
        .rotation = 0u,
    };

    for (uint8_t y = 0u; y < 4u; y++) {
        for (uint8_t x = 0u; x < 4u; x++) {
            if (block_shape_cell(piece_type, 0u, x, y)) {
                if (x < minimum_x) {
                    minimum_x = x;
                }
                if (x > maximum_x) {
                    maximum_x = x;
                }
                if (y < minimum_y) {
                    minimum_y = y;
                }
                if (y > maximum_y) {
                    maximum_y = y;
                }
            }
        }
    }
    block_stack_draw_piece(
        &preview,
        (uint8_t)(BLOCK_PREVIEW_X + 1u +
                  (BLOCK_PREVIEW_WIDTH - 2u -
                   (maximum_x - minimum_x + 1u) * BLOCK_PREVIEW_CELL_SIZE) /
                      2u -
                  minimum_x * BLOCK_PREVIEW_CELL_SIZE),
        (uint8_t)(BLOCK_PREVIEW_Y + 1u +
                  (BLOCK_PREVIEW_HEIGHT - 2u -
                   (maximum_y - minimum_y + 1u) * BLOCK_PREVIEW_CELL_SIZE) /
                      2u -
                  minimum_y * BLOCK_PREVIEW_CELL_SIZE),
        BLOCK_PREVIEW_CELL_SIZE);
}

static void block_stack_draw_current_piece(const block_stack_game_t *game) {
    for (uint8_t y = 0u; y < 4u; y++) {
        for (uint8_t x = 0u; x < 4u; x++) {
            int8_t board_x;
            int8_t board_y;

            if (!block_shape_cell(game->current_piece.type,
                                  game->current_piece.rotation, x, y)) {
                continue;
            }
            board_x = (int8_t)(game->current_piece.x + x);
            board_y = (int8_t)(game->current_piece.y + y);
            if ((board_x >= 0) && (board_x < (int8_t)BLOCK_BOARD_WIDTH) &&
                (board_y >= 0) && (board_y < (int8_t)BLOCK_BOARD_HEIGHT)) {
                block_stack_draw_cell(
                    (uint8_t)(BLOCK_GRID_X + board_x * BLOCK_CELL_SIZE),
                    (uint8_t)(BLOCK_GRID_Y + board_y * BLOCK_CELL_SIZE),
                    BLOCK_CELL_SIZE);
            }
        }
    }
}

void block_stack_game_draw(const void *context) {
    const block_stack_game_t *game =
        (const block_stack_game_t *)context;

    if (game == NULL) {
        return;
    }
    game_display_clear();
    block_stack_draw_wall(BLOCK_LEFT_WALL_X, BLOCK_LEFT_WALL_WIDTH);
    block_stack_draw_wall(BLOCK_RIGHT_WALL_X, BLOCK_RIGHT_WALL_WIDTH);
    game_gfx_draw_rect(BLOCK_BOARD_X, BLOCK_BOARD_Y,
                       BLOCK_BOARD_FRAME_WIDTH,
                       BLOCK_FRAME_HEIGHT, true);
    game_gfx_draw_rect(BLOCK_PANEL_X, BLOCK_BOARD_Y, BLOCK_PANEL_WIDTH,
                       BLOCK_FRAME_HEIGHT, true);

    for (uint8_t y = 0u; y < BLOCK_BOARD_HEIGHT; y++) {
        for (uint8_t x = 0u; x < BLOCK_BOARD_WIDTH; x++) {
            if ((game->board[y] & (1u << x)) != 0u) {
                block_stack_draw_cell(
                    (uint8_t)(BLOCK_GRID_X + x * BLOCK_CELL_SIZE),
                    (uint8_t)(BLOCK_GRID_Y + y * BLOCK_CELL_SIZE),
                    BLOCK_CELL_SIZE);
            }
        }
    }
    if (!game->game_over) {
        block_stack_draw_current_piece(game);
    }

    game_gfx_draw_rect(BLOCK_PREVIEW_X, BLOCK_PREVIEW_Y, BLOCK_PREVIEW_WIDTH,
                       BLOCK_PREVIEW_HEIGHT, true);
    block_stack_draw_preview(game->next_piece);
    game_gfx_draw_text(block_stack_center_x(5u * BLOCK_FONT_WIDTH),
                       BLOCK_SCORE_LABEL_Y, "SCORE", 1u, true, false);
    game_gfx_draw_number(
        block_stack_center_x(block_stack_decimal_width(game->score)),
        BLOCK_SCORE_VALUE_Y, game->score, 1u, true, false);
    game_gfx_draw_text(block_stack_center_x(BLOCK_LEVEL_ROW_WIDTH),
                       BLOCK_LEVEL_ROW_Y, "LV", 1u, true, false);
    if (game->level < 10u) {
        game_gfx_draw_text(
            (uint8_t)(block_stack_center_x(BLOCK_LEVEL_ROW_WIDTH) +
                      3u * BLOCK_FONT_WIDTH),
            BLOCK_LEVEL_ROW_Y, "0", 1u, true, false);
        game_gfx_draw_number(
            (uint8_t)(block_stack_center_x(BLOCK_LEVEL_ROW_WIDTH) +
                      4u * BLOCK_FONT_WIDTH),
            BLOCK_LEVEL_ROW_Y, game->level, 1u, true, false);
    } else {
        game_gfx_draw_number(
            (uint8_t)(block_stack_center_x(BLOCK_LEVEL_ROW_WIDTH) +
                      3u * BLOCK_FONT_WIDTH),
            BLOCK_LEVEL_ROW_Y, game->level, 1u, true, false);
    }

    if (game->game_over) {
        game_display_fill_rect(18u, 21u, 40u, 22u, false);
        game_gfx_draw_text(26u, 23u, "GAME", 1u, true, false);
        game_gfx_draw_text(26u, 33u, "OVER", 1u, true, false);
    }
}
