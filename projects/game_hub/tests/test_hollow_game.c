#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "block_stack_game.h"
#include "dino_game.h"
#include "game_hal.h"
#include "game_manager.h"
#include "game_registry.h"
#include "hollow_game.h"
#include "math_rush_game.h"
#include "map3d_game.h"
#include "ufo_game.h"

static uint32_t drawn_pixels;
static uint8_t last_bitmap_y;
static uint32_t score_draw_count;

void game_display_clear(void) {
    drawn_pixels = 0u;
    last_bitmap_y = 0u;
    score_draw_count = 0u;
}

void game_display_draw_pixel(uint8_t x, uint8_t y, bool on) {
    (void)x;
    (void)y;
    if (on) {
        drawn_pixels++;
    }
}

void game_display_draw_bitmap(uint8_t x, uint8_t y, const uint8_t *bitmap,
                              uint8_t width, uint8_t height, bool on) {
    (void)x;
    (void)bitmap;
    (void)width;
    (void)height;
    if (on) {
        drawn_pixels++;
        if ((width == 8u) && (height == 8u)) {
            last_bitmap_y = y;
        }
    }
}

void game_gfx_draw_text(uint8_t x, uint8_t y, const char *text,
                        uint8_t size, bool foreground, bool background) {
    (void)x;
    (void)y;
    (void)text;
    (void)size;
    (void)foreground;
    (void)background;
    drawn_pixels++;
}

void game_gfx_draw_number(uint8_t x, uint8_t y, uint32_t value,
                          uint8_t size, bool foreground, bool background) {
    (void)x;
    (void)y;
    (void)value;
    (void)size;
    (void)foreground;
    (void)background;
    score_draw_count++;
}

void game_gfx_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                        bool on) {
    game_display_fill_rect(x, y, width, height, on);
}

void game_display_fill_rect(uint8_t x, uint8_t y, uint8_t width,
                            uint8_t height, bool on) {
    (void)x;
    (void)y;
    if (on) {
        drawn_pixels += (uint32_t)width * height;
    }
}

void game_buzzer_off(void) {
}

void game_buzzer_tone(uint16_t frequency_hz) {
    (void)frequency_hz;
}

void hollow_audio_stop(void) {
}

static void set_open_cave(hollow_game_t *game) {
    for (uint8_t column = 0u; column < HOLLOW_COLUMN_COUNT; column++) {
        game->columns[column].top = 28u;
        game->columns[column].bottom = 40u;
    }
}

static void test_ufo_player_hitbox(void) {
    ufo_game_t game;

    ufo_game_init(&game, 1234u);
    game.state = UFO_STATE_PLAYING;
    game.enemy_shots[0].x = 13;
    game.enemy_shots[0].y = (int8_t)game.player_y;
    game.enemy_shots[0].active = true;
    ufo_game_update(&game, 0u, 33u);
    assert(game.hp == 3u);

    ufo_game_init(&game, 1234u);
    game.state = UFO_STATE_PLAYING;
    game.enemy_shots[0].x = 13;
    game.enemy_shots[0].y = (int8_t)(game.player_y + 2u);
    game.enemy_shots[0].active = true;
    ufo_game_update(&game, 0u, 33u);
    assert(game.hp == 2u);
    assert(!game.enemy_shots[0].active);

    /* The transparent center of the current UFO bitmap is not a hit. */
    ufo_game_init(&game, 1234u);
    game.state = UFO_STATE_PLAYING;
    game.enemy_shots[0].x = 13;
    game.enemy_shots[0].y = (int8_t)(game.player_y + 4u);
    game.enemy_shots[0].active = true;
    ufo_game_update(&game, 0u, 33u);
    assert(game.hp == 3u);
    assert(game.enemy_shots[0].active);
}

static void test_ufo_context_bounds(void) {
    struct {
        uint32_t before;
        ufo_game_t game;
        uint32_t after;
    } guarded = {
        .before = 0x13579BDFu,
        .after = 0x2468ACE0u,
    };

    ufo_game_init(&guarded.game, 1234u);
    ufo_game_update(&guarded.game, GAME_INPUT_MODE, 16u);
    ufo_game_update(&guarded.game, 0u, 16u);
    for (uint32_t frame = 0u; frame < 12000u; frame++) {
        uint8_t input = ((frame / 90u) & 1u) ?
                        GAME_INPUT_UP : GAME_INPUT_DOWN;

        if ((frame % 4u) == 0u) {
            input |= GAME_INPUT_MODE;
        }
        ufo_game_update(&guarded.game, input, 16u);
        ufo_game_draw(&guarded.game);
        if (guarded.game.state == UFO_STATE_GAME_OVER) {
            ufo_game_update(&guarded.game, 0u, 16u);
            ufo_game_update(&guarded.game, GAME_INPUT_UP, 16u);
            ufo_game_update(&guarded.game, 0u, 16u);
            ufo_game_update(&guarded.game, GAME_INPUT_MODE, 16u);
        }
        assert(guarded.before == 0x13579BDFu);
        assert(guarded.after == 0x2468ACE0u);
        assert(guarded.game.player_y >= 8u);
        assert(guarded.game.player_y <= 48u);
    }
}

static void test_map3d_exploration(void) {
    map3d_game_t game;
    uint16_t start_x;

    assert(sizeof(game) <= GAME_MANAGER_CONTEXT_SIZE);
    map3d_game_init(&game, 1234u);
    assert(game.player_x == 2176u);
    assert(game.player_y == 2176u);
    start_x = game.player_x;
    map3d_game_update(&game, GAME_INPUT_UP, 32u);
    assert(game.player_x > start_x);
    map3d_game_update(&game, GAME_INPUT_DOWN, 16u);
    assert(game.angle == 255u);
    map3d_game_update(&game, GAME_INPUT_MODE, 48u);
    assert(game.angle == 0u);
    map3d_game_update(&game, GAME_INPUT_DOWN | GAME_INPUT_MODE, 1999u);
    assert(!game.map_visible);
    map3d_game_update(&game, GAME_INPUT_DOWN | GAME_INPUT_MODE, 1u);
    assert(game.map_visible);
    map3d_game_update(&game, GAME_INPUT_DOWN | GAME_INPUT_MODE, 2000u);
    assert(game.map_visible);
    map3d_game_update(&game, 0u, 16u);
    map3d_game_update(&game, GAME_INPUT_UP, 16u);
    assert(!game.map_visible);
    map3d_game_draw(&game);
    assert(drawn_pixels > 0u);
}

static void test_block_stack_logic(void) {
    static const uint32_t expected_score[] = {0u, 100u, 300u, 500u, 800u};
    block_stack_game_t game;
    block_piece_t piece;

    assert(sizeof(game) <= GAME_MANAGER_CONTEXT_SIZE);
    block_stack_game_init(&game, 0x12345678u);
    assert(!game.game_over);
    assert(game.fall_interval_ms == 500u);
    assert(game.level == 1u);
    {
        int8_t start_x = game.current_piece.x;

        block_stack_game_update(&game, GAME_INPUT_DOWN, 16u);
        assert(game.current_piece.x == start_x - 1);
        block_stack_game_update(&game, GAME_INPUT_DOWN, 16u);
        assert(game.current_piece.x == start_x - 1);
        block_stack_game_update(&game, 0u, 16u);
        block_stack_game_update(&game, GAME_INPUT_DOWN, 16u);
        assert(game.current_piece.x == start_x - 2);
    }

    memset(&piece, 0, sizeof(piece));
    piece.type = BLOCK_PIECE_I;
    piece.x = 6;
    piece.y = 0;
    assert(block_stack_piece_fits(&game, &piece));
    piece.x = 7;
    assert(!block_stack_piece_fits(&game, &piece));
    piece.x = -1;
    assert(!block_stack_piece_fits(&game, &piece));
    piece.x = 3;
    piece.y = 14;
    assert(!block_stack_piece_fits(&game, &piece));
    piece.y = 0;
    game.board[1] = 1u << 3;
    assert(!block_stack_piece_fits(&game, &piece));

    for (uint8_t count = 1u; count <= 4u; count++) {
        memset(game.board, 0, sizeof(game.board));
        game.score = 0u;
        game.lines_cleared = 0u;
        game.fall_interval_ms = 500u;
        game.board[10] = 1u;
        for (uint8_t row = 0u; row < count; row++) {
            game.board[BLOCK_BOARD_HEIGHT - 1u - row] =
                (1u << BLOCK_BOARD_WIDTH) - 1u;
        }
        assert(block_stack_clear_lines(&game) == count);
        assert(game.score == expected_score[count]);
        assert(game.lines_cleared == count);
        assert(game.board[10u + count] == 1u);
    }

    memset(game.board, 0, sizeof(game.board));
    game.score = 400u;
    game.board[BLOCK_BOARD_HEIGHT - 1u] =
        (1u << BLOCK_BOARD_WIDTH) - 1u;
    assert(block_stack_clear_lines(&game) == 1u);
    assert(game.score == 500u);
    assert(game.level == 2u);
    assert(game.fall_interval_ms == 450u);

    game.score = 3900u;
    game.board[BLOCK_BOARD_HEIGHT - 1u] =
        (1u << BLOCK_BOARD_WIDTH) - 1u;
    assert(block_stack_clear_lines(&game) == 1u);
    assert(game.score == 4000u);
    assert(game.level == 9u);
    assert(game.fall_interval_ms == 100u);

    memset(game.board, 0, sizeof(game.board));
    game.current_piece.type = BLOCK_PIECE_I;
    game.current_piece.rotation = 1u;
    game.current_piece.x = -1;
    game.current_piece.y = 0;
    game.input_previous = 0u;
    block_stack_game_update(&game, GAME_INPUT_MODE, 16u);
    assert(game.current_piece.rotation == 1u);

    game.game_over = true;
    game.input_previous = 0u;
    block_stack_game_update(&game, GAME_INPUT_MODE, 16u);
    assert(!game.game_over);
    game.game_over = true;
    game.input_previous = 0u;
    block_stack_game_update(&game, GAME_INPUT_UP, 16u);
    assert(game.return_to_menu);
}

static void test_block_stack_context_bounds(void) {
    struct {
        uint32_t before;
        block_stack_game_t game;
        uint32_t after;
    } guarded = {
        .before = 0x89ABCDEFu,
        .after = 0x10293847u,
    };
    uint32_t random = 0x13572468u;

    block_stack_game_init(&guarded.game, random);
    for (uint32_t frame = 0u; frame < 20000u; frame++) {
        uint8_t input = 0u;

        random ^= random << 13;
        random ^= random >> 17;
        random ^= random << 5;
        if ((frame & 1u) != 0u) {
            input = (uint8_t)(1u << (random % 3u));
        }
        block_stack_game_update(&guarded.game, input, 33u);
        if (guarded.game.game_over) {
            block_stack_game_update(&guarded.game, 0u, 16u);
            block_stack_game_update(&guarded.game, GAME_INPUT_MODE, 16u);
        }
        assert(guarded.before == 0x89ABCDEFu);
        assert(guarded.after == 0x10293847u);
        for (uint8_t row = 0u; row < BLOCK_BOARD_HEIGHT; row++) {
            assert((guarded.game.board[row] &
                    ~((1u << BLOCK_BOARD_WIDTH) - 1u)) == 0u);
        }
        if (!guarded.game.game_over) {
            assert(block_stack_piece_fits(
                &guarded.game, &guarded.game.current_piece));
        }
    }
}

static void test_dino_runner_logic(void) {
    dino_game_t game;

    assert(sizeof(game) <= GAME_MANAGER_CONTEXT_SIZE);
    dino_game_init(&game, 0x12345678u);
    assert(game.state == DINO_STATE_READY);
    assert(game.score == 0u);

    dino_game_update(&game, GAME_INPUT_MODE, 16u);
    assert(game.state == DINO_STATE_RUNNING);
    dino_game_update(&game, 0u, 16u);
    dino_game_update(&game, GAME_INPUT_MODE, 16u);
    assert(game.player_velocity_q4 < 0);
    assert(game.player_y_q4 < 37 * 16);

    dino_game_init(&game, 0x12345678u);
    dino_game_update(&game, GAME_INPUT_MODE, 16u);
    dino_game_update(&game, 0u, 16u);
    dino_game_update(&game, 0u, 100u);
    dino_game_update(&game, 0u, 16u);
    assert(game.score >= 1u);
    assert(game.high_score == game.score);

    /* A normal MODE jump from 40 px away must clear the largest cactus. */
    dino_game_init(&game, 0x12345678u);
    dino_game_update(&game, GAME_INPUT_MODE, 16u);
    dino_game_update(&game, 0u, 16u);
    game.spawn_remaining_q4 = 30000;
    game.obstacles[0].active = true;
    game.obstacles[0].type = DINO_OBSTACLE_CACTUS_BIG_BIG;
    game.obstacles[0].x_q4 = 40 * 16;
    dino_game_update(&game, GAME_INPUT_MODE, 16u);
    for (uint8_t frame = 0u; frame < 46u; frame++) {
        dino_game_update(&game, 0u, 16u);
    }
    assert(game.state == DINO_STATE_RUNNING);
    assert(game.obstacles[0].x_q4 < 0);

    dino_game_init(&game, 0x12345678u);
    dino_game_update(&game, GAME_INPUT_MODE, 16u);
    dino_game_update(&game, 0u, 16u);
    game.obstacles[0].active = true;
    game.obstacles[0].type = DINO_OBSTACLE_CACTUS_SMALL_BIG;
    game.obstacles[0].x_q4 = 12 * 16;
    game.player_y_q4 = 37 * 16;
    game.player_velocity_q4 = 0;
    dino_game_update(&game, 0u, 16u);
    assert(game.state == DINO_STATE_GAME_OVER);
    dino_game_update(&game, GAME_INPUT_MODE, 16u);
    assert(game.return_to_menu);

    dino_game_init(&game, 0x12345678u);
    game.state = DINO_STATE_GAME_OVER;
    game.high_score = 42u;
    dino_game_update(&game, GAME_INPUT_UP, 16u);
    assert(game.state == DINO_STATE_RUNNING);
    assert(game.high_score == 42u);
    assert(!game.return_to_menu);

    dino_game_draw(&game);
    assert(drawn_pixels > 0u);
}

static void test_dino_runner_context_bounds(void) {
    struct {
        uint32_t before;
        dino_game_t game;
        uint32_t after;
    } guarded = {
        .before = 0xA1B2C3D4u,
        .after = 0x5E6F7081u,
    };
    uint32_t random = 0x31415926u;

    dino_game_init(&guarded.game, random);
    dino_game_update(&guarded.game, GAME_INPUT_MODE, 16u);
    for (uint32_t frame = 0u; frame < 20000u; frame++) {
        uint8_t input = 0u;

        random ^= random << 13;
        random ^= random >> 17;
        random ^= random << 5;
        if ((frame & 3u) == 0u) {
            input = (uint8_t)(1u << (random % 3u));
        }
        dino_game_update(&guarded.game, input, 17u);
        if (guarded.game.state == DINO_STATE_GAME_OVER) {
            dino_game_update(&guarded.game, 0u, 16u);
            dino_game_update(&guarded.game, GAME_INPUT_UP, 16u);
        }
        assert(guarded.before == 0xA1B2C3D4u);
        assert(guarded.after == 0x5E6F7081u);
    }
}

int main(void) {
    hollow_game_t game;
    game_manager_t manager;

    test_ufo_player_hitbox();
    test_ufo_context_bounds();
    test_map3d_exploration();
    test_block_stack_logic();
    test_block_stack_context_bounds();
    test_dino_runner_logic();
    test_dino_runner_context_bounds();
    assert(game_registry_count() == 9u);
    assert(game_registry_get(0u) != NULL);
    assert(game_registry_get(1u) != NULL);
    assert(game_registry_get(2u) != NULL);
    assert(game_registry_get(3u) != NULL);
    assert(game_registry_get(4u) != NULL);
    assert(game_registry_get(5u) != NULL);
    assert(game_registry_get(6u) != NULL);
    assert(game_registry_get(7u) != NULL);
    assert(game_registry_get(8u) != NULL);
    assert(game_registry_get(9u) == NULL);
    assert(strcmp(game_registry_get(0u)->name, "BIKE RACER") == 0);
    assert(strcmp(game_registry_get(2u)->name, "HOLLOW") == 0);

    game_manager_init(&manager);
    assert(manager.mode == GAME_MANAGER_MENU);
    game_manager_update(&manager, GAME_INPUT_DOWN, 16u);
    assert(manager.selected_index == 1u);
    game_manager_update(&manager, 0u, 16u);
    game_manager_update(&manager, GAME_INPUT_UP, 16u);
    assert(manager.selected_index == 0u);
    game_manager_update(&manager, 0u, 16u);
    game_manager_update(&manager, GAME_INPUT_MODE, 16u);
    assert(manager.mode == GAME_MANAGER_RUNNING);
    assert(manager.active_game == game_registry_get(0u));
    assert(manager.input_locked);

    /* The MODE press that starts a game must not leak into the game context. */
    game_manager_update(&manager, GAME_INPUT_MODE, 16u);
    game_manager_update(&manager, 0u, 16u);
    assert(!manager.input_locked);
    game_manager_update(&manager, 0u, GAME_MANAGER_AUTO_SNAKE_IDLE_MS + 1u);
    assert(manager.mode == GAME_MANAGER_RUNNING);
    assert(manager.active_game == game_registry_get(0u));
    for (uint8_t update = 0u; update < 120u; update++) {
        game_manager_update(&manager, GAME_INPUT_MODE, 16u);
    }
    assert(manager.mode == GAME_MANAGER_RUNNING);
    assert(manager.active_game == game_registry_get(0u));

    /* Holding DOWN repeats only after the initial navigation delay. */
    game_manager_init(&manager);
    game_manager_update(&manager, GAME_INPUT_DOWN, 16u);
    assert(manager.selected_index == 1u);
    for (uint8_t update = 1u;
         update < GAME_MANAGER_REPEAT_DELAY_UPDATES - 1u; update++) {
        game_manager_update(&manager, GAME_INPUT_DOWN, 16u);
        assert(manager.selected_index == 1u);
    }
    game_manager_update(&manager, GAME_INPUT_DOWN, 16u);
    assert(manager.selected_index == 2u);
    game_manager_update(&manager, 0u, 16u);
    game_manager_update(&manager, GAME_INPUT_UP, 16u);
    assert(manager.selected_index == 1u);
    game_manager_update(&manager, 0u, 16u);

    /* Holding MODE is delivered to Math Rush and never exits the game. */
    assert(manager.selected_index == 1u);
    game_manager_update(&manager, 0u, 16u);
    game_manager_update(&manager, GAME_INPUT_MODE, 16u);
    assert(manager.active_game == game_registry_get(1u));
    game_manager_update(&manager, 0u, 16u);
    for (uint8_t update = 0u; update < 120u; update++) {
        game_manager_update(&manager, GAME_INPUT_MODE, 16u);
    }
    assert(manager.mode == GAME_MANAGER_RUNNING);
    assert(manager.active_game == game_registry_get(1u));

    hollow_game_init(&game);
    assert(game.starting);
    assert((hollow_game_take_events(&game) & HOLLOW_EVENT_START) != 0u);
    assert(hollow_game_take_events(&game) == HOLLOW_EVENT_NONE);
    assert(game.counter == 120u);
    assert(game.player_column == 1u);

    for (uint16_t frame = 0u; frame < 120u; frame++) {
        hollow_game_update(&game, 0u);
    }
    assert(!game.starting);
    assert(game.cave_phase == 2u);

    set_open_cave(&game);
    game.player_x = 20u;
    game.player_column = 5u;
    game.player_move = 0u;
    hollow_game_update(&game, GAME_INPUT_DOWN);
    assert(!game.player_direction);
    assert(game.player_column == 4u);
    assert(game.player_x == 19u);
    assert(game.player_move == 7u);
    assert((hollow_game_take_events(&game) & HOLLOW_EVENT_MOVE) != 0u);

    game.input_previous = 0u;
    game.player_x = 56u;
    game.player_column = 5u;
    game.player_move = 0u;
    game.cave_offset = 0u;
    game.score = 0u;
    hollow_game_update(&game, GAME_INPUT_UP);
    assert(game.player_direction);
    assert(game.player_x == 56u);
    assert(game.cave_offset == 1u);
    assert(game.score == 1u);

    set_open_cave(&game);
    game.starting = false;
    game.game_over = false;
    game.cave_phase = 1022u;
    game.columns[game.player_column].top = 37u;
    game.columns[game.player_column].bottom = 40u;
    hollow_game_update(&game, 0u);
    assert(game.cave_phase == 0u);
    assert(game.game_over);
    assert(game.counter == 480u);
    assert((hollow_game_take_events(&game) & HOLLOW_EVENT_GAME_OVER) != 0u);

    game.input_previous = 0u;
    hollow_game_update(&game, GAME_INPUT_MODE);
    assert(game.return_to_menu);
    assert(game.game_over);

    hollow_game_draw(&game);
    assert(drawn_pixels > 0u);

    game.starting = false;
    game.game_over = false;
    game.player_move = 8u;
    game.player_jump = 8;
    game.columns[game.player_column].top = 20u;
    game.columns[game.player_column].bottom = 36u;
    game.cave_base_top = 0;
    game.cave_base_bottom = 0;
    hollow_game_draw(&game);
    assert(last_bitmap_y == 31u);

    game.player_move = 0u;
    hollow_game_draw(&game);
    assert(last_bitmap_y == 23u);
    assert(score_draw_count == 1u);

    /* Apply the render offset before clipping to the bottom of the screen. */
    game.columns[game.player_column].top = 40u;
    game.columns[game.player_column].bottom = 56u;
    game.cave_base_top = 0;
    game.cave_base_bottom = 16;
    hollow_game_draw(&game);
    assert(last_bitmap_y == 56u);

    puts("hollow_game: all tests passed");
    return 0;
}
