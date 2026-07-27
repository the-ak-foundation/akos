#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "game_hal.h"

static bool display_pixels[GAME_DISPLAY_HEIGHT][GAME_DISPLAY_WIDTH];
static uint32_t display_clear_count;

void game_display_clear(void) {
    memset(display_pixels, 0, sizeof(display_pixels));
    display_clear_count++;
}

void game_display_draw_pixel(uint8_t x, uint8_t y, bool on) {
    assert(x < GAME_DISPLAY_WIDTH);
    assert(y < GAME_DISPLAY_HEIGHT);
    display_pixels[y][x] = on;
}

void game_display_fill_rect(uint8_t x, uint8_t y, uint8_t width,
                            uint8_t height, bool on) {
    assert((uint16_t)x + width <= GAME_DISPLAY_WIDTH);
    assert((uint16_t)y + height <= GAME_DISPLAY_HEIGHT);
    for (uint8_t row = 0u; row < height; row++) {
        for (uint8_t column = 0u; column < width; column++) {
            display_pixels[y + row][x + column] = on;
        }
    }
}

#include "../app/auto_snake/auto_snake_game.c"

static void test_hamiltonian_cycle(void) {
    bool visited[512] = {false};
    bool index_seen[512] = {false};
    auto_snake_cell_t current = 0u;

    for (uint16_t step = 0u; step < 512u; step++) {
        auto_snake_cell_t next;
        uint8_t x;
        uint8_t y;
        uint8_t next_x;
        uint8_t next_y;
        uint8_t distance;

        assert(current < 512u);
        assert(!visited[current]);
        visited[current] = true;
        assert(auto_snake_hamiltonian_index(current) < 512u);
        assert(!index_seen[auto_snake_hamiltonian_index(current)]);
        index_seen[auto_snake_hamiltonian_index(current)] = true;
        next = auto_snake_hamiltonian_next(current);
        assert(auto_snake_hamiltonian_index(next) ==
               ((auto_snake_hamiltonian_index(current) + 1u) & 0x01ffu));
        x = auto_snake_cell_x(current);
        y = auto_snake_cell_y(current);
        next_x = auto_snake_cell_x(next);
        next_y = auto_snake_cell_y(next);
        distance = (uint8_t)(
            (x > next_x ? x - next_x : next_x - x) +
            (y > next_y ? y - next_y : next_y - y));
        assert(next_x < AUTO_SNAKE_GRID_WIDTH);
        assert(next_y < AUTO_SNAKE_GRID_HEIGHT);
        assert(distance == 1u);
        current = next;
    }
    assert(current == 0u);
    for (uint16_t cell = 0u; cell < 512u; cell++) {
        assert(visited[cell]);
        assert(index_seen[cell]);
    }
}

static auto_snake_cell_t test_cell_at_cycle_index(uint16_t target) {
    auto_snake_cell_t cell = 0u;

    for (uint16_t index = 0u; index < target; index++) {
        cell = auto_snake_hamiltonian_next(cell);
    }
    return cell;
}

static void test_make_contiguous_snake(auto_snake_t *snake,
                                       uint16_t head_order,
                                       uint8_t length) {
    memset(snake, 0, sizeof(*snake));
    snake->length = length;
    snake->head_index = (uint8_t)(length - 1u);
    for (uint8_t index = 0u; index < length; index++) {
        uint16_t order = (uint16_t)(
            (head_order + 512u - length + 1u + index) & 0x01ffu);

        snake->body[index] = test_cell_at_cycle_index(order);
    }
}

static void test_safe_shortcut(void) {
    auto_snake_t snake;
    auto_snake_cell_t head = auto_snake_make_cell(10u, 5u);
    auto_snake_cell_t food = auto_snake_make_cell(10u, 6u);
    auto_snake_cell_t default_cell = auto_snake_hamiltonian_next(head);
    auto_snake_cell_t selected;

    test_make_contiguous_snake(
        &snake, auto_snake_hamiltonian_index(head), 5u);
    selected = auto_snake_choose_next(&snake, NULL, food);
    assert(selected == food);
    assert(selected != default_cell);
    assert(auto_snake_manhattan_distance(head, selected) == 1u);
    assert(!auto_snake_contains(&snake, selected));

    memset(&snake, 0, sizeof(snake));
    snake.length = 1u;
    snake.body[0] = head;
    selected = auto_snake_choose_next(&snake, NULL, food);
    assert(selected == default_cell);
}

static void test_initial_state_and_food(void) {
    auto_snake_game_t game;

    auto_snake_game_init(&game, 0x12345678u);
    assert(sizeof(game) <= 512u);
    assert(game.snake.length == AUTO_SNAKE_INITIAL_LENGTH);
    assert(game.snake.head_index == AUTO_SNAKE_INITIAL_LENGTH - 1u);
    assert(!auto_snake_contains(&game.snake, game.food));
    for (uint8_t repeat = 0u; repeat < 100u; repeat++) {
        auto_snake_place_food(&game);
        assert(game.food < 512u);
        assert(!auto_snake_contains(&game.snake, game.food));
    }
}

static void test_circular_wrap_and_bite(void) {
    auto_snake_game_t game;
    auto_snake_cell_t current = 0u;
    auto_snake_cell_t removed_cells[50];

    memset(&game, 0, sizeof(game));
    game.random_state = 0x87654321u;
    game.move_interval_ms = AUTO_SNAKE_INITIAL_MOVE_MS;
    game.snake.length = 5u;
    game.snake.head_index = 59u;
    for (uint8_t offset = 4u; offset > 0u; offset--) {
        game.snake.body[(uint8_t)(59u - offset)] = current;
        current = auto_snake_hamiltonian_next(current);
    }
    game.snake.body[59] = current;
    game.food = 511u;
    auto_snake_move_playing(&game);
    assert(game.snake.head_index == 0u);
    assert(game.snake.length == 5u);
    assert(game.erased_tail_valid);

    memset(&game, 0, sizeof(game));
    game.random_state = 0x13572468u;
    game.move_interval_ms = AUTO_SNAKE_INITIAL_MOVE_MS;
    game.snake.length = AUTO_SNAKE_MAX_LENGTH - 1u;
    game.snake.head_index = AUTO_SNAKE_MAX_LENGTH - 2u;
    current = 0u;
    for (uint8_t index = 0u; index < game.snake.length; index++) {
        game.snake.body[index] = current;
        if (index < 50u) {
            removed_cells[index] = current;
        }
        current = auto_snake_hamiltonian_next(current);
    }
    game.food = current;
    game.food_available = true;
    auto_snake_move_playing(&game);
    assert(game.snake.length == AUTO_SNAKE_MAX_LENGTH);
    assert(game.state == AUTO_SNAKE_STATE_SEEK_BITE);
    assert(game.bite_cell == auto_snake_body_at_offset(
        &game.snake, AUTO_SNAKE_BITE_OFFSET));
    assert(!game.food_dirty);
    assert(game.redraw_all);

    for (uint16_t step = 0u;
         (step < 4096u) &&
         (game.state == AUTO_SNAKE_STATE_SEEK_BITE); step++) {
        auto_snake_cell_t old_head =
            game.snake.body[game.snake.head_index];

        auto_snake_seek_bite_step(&game);
        if (game.state == AUTO_SNAKE_STATE_SEEK_BITE) {
            auto_snake_cell_t new_head =
                game.snake.body[game.snake.head_index];

            assert(auto_snake_manhattan_distance(
                old_head, new_head) == 1u);
        }
        if (game.state == AUTO_SNAKE_STATE_SEEK_BITE) {
            assert(game.bite_cell == auto_snake_body_at_offset(
                &game.snake, AUTO_SNAKE_BITE_OFFSET));
        }
    }
    if (game.state != AUTO_SNAKE_STATE_BITE_ANIMATION) {
        fprintf(stderr,
                "bite seek stuck: steps=%u fallbacks=%u head=%u bite=%u\n",
                game.bite_seek_steps, game.bite_seek_fallbacks,
                game.snake.body[game.snake.head_index],
                game.bite_cell);
    }
    assert(game.state == AUTO_SNAKE_STATE_BITE_ANIMATION);
    assert(auto_snake_manhattan_distance(
        game.snake.body[game.snake.head_index],
        game.bite_cell) == 1u);
    for (uint8_t offset = AUTO_SNAKE_BITE_OFFSET;
         offset < game.snake.length; offset++) {
        removed_cells[offset - AUTO_SNAKE_BITE_OFFSET] =
            auto_snake_body_at_offset(&game.snake, offset);
    }
    for (uint8_t frame = 0u;
         frame < AUTO_SNAKE_BITE_FRAME_COUNT; frame++) {
        auto_snake_game_update(
            &game, 0u, AUTO_SNAKE_BITE_FRAME_MS);
    }
    assert(game.state == AUTO_SNAKE_STATE_CUT_BODY);
    assert(game.snake.length == AUTO_SNAKE_MAX_LENGTH);
    auto_snake_game_update(&game, 0u, 0u);
    assert(game.state == AUTO_SNAKE_STATE_PLAYING);
    assert(game.snake.length == AUTO_SNAKE_KEEP_LENGTH);
    assert(game.bite_count == 1u);
    assert(game.cut_redraw);
    assert(game.detached.active);
    assert(game.detached.visible);
    assert(game.detached.state ==
           AUTO_SNAKE_DETACHED_BLINKING);
    assert(game.detached.length ==
           AUTO_SNAKE_MAX_DETACHED_LENGTH);
    assert(game.detached.remaining ==
           AUTO_SNAKE_MAX_DETACHED_LENGTH);
    for (uint8_t index = 0u;
         index < AUTO_SNAKE_MAX_DETACHED_LENGTH; index++) {
        assert(game.detached.cells[index] ==
               removed_cells[index]);
        assert(auto_snake_detached_contains(
            &game.detached, removed_cells[index]));
    }
    assert(!auto_snake_contains(&game.snake, game.food));
    assert(!auto_snake_detached_contains(
        &game.detached, game.food));
    display_clear_count = 0u;
    auto_snake_game_draw(&game);
    assert(display_clear_count == 0u);
    assert(!game.cut_redraw);

    auto_snake_update_detached(
        &game.detached, AUTO_SNAKE_DETACHED_BLINK_MS);
    assert(!game.detached.visible);
    assert(auto_snake_detached_contains(
        &game.detached, removed_cells[0]));
    auto_snake_game_draw(&game);
    assert(!display_pixels[
        auto_snake_cell_y(removed_cells[0]) << 2][
        auto_snake_cell_x(removed_cells[0]) << 2]);

    auto_snake_update_detached(
        &game.detached,
        AUTO_SNAKE_DETACHED_DURATION_MS -
        AUTO_SNAKE_DETACHED_BLINK_MS);
    assert(game.detached.state ==
           AUTO_SNAKE_DETACHED_DISAPPEARING);
    assert(game.detached.visible);
    assert(game.detached.remaining ==
           AUTO_SNAKE_MAX_DETACHED_LENGTH);
    auto_snake_update_detached(
        &game.detached, AUTO_SNAKE_DETACHED_REMOVE_MS);
    assert(game.detached.remaining ==
           AUTO_SNAKE_MAX_DETACHED_LENGTH - 1u);
    assert(!auto_snake_detached_contains(
        &game.detached,
        removed_cells[AUTO_SNAKE_MAX_DETACHED_LENGTH - 1u]));
    assert(auto_snake_detached_contains(
        &game.detached, removed_cells[0]));
    for (uint8_t remaining =
             AUTO_SNAKE_MAX_DETACHED_LENGTH - 1u;
         remaining > 0u; remaining--) {
        auto_snake_update_detached(
            &game.detached, AUTO_SNAKE_DETACHED_REMOVE_MS);
    }
    assert(!game.detached.active);
    assert(!game.detached.visible);
    assert(game.detached.remaining == 0u);
    assert(game.detached.state == AUTO_SNAKE_DETACHED_NONE);
    auto_snake_game_draw(&game);
    for (uint8_t removed = 0u;
         removed < AUTO_SNAKE_MAX_DETACHED_LENGTH; removed++) {
        auto_snake_cell_t removed_cell = removed_cells[removed];
        uint8_t pixel_x =
            (uint8_t)(auto_snake_cell_x(removed_cell) << 2);
        uint8_t pixel_y =
            (uint8_t)(auto_snake_cell_y(removed_cell) << 2);

        if (auto_snake_contains(&game.snake, removed_cell) ||
            (game.food_available && (game.food == removed_cell))) {
            continue;
        }
        for (uint8_t row = 0u;
             row < AUTO_SNAKE_SEGMENT_SIZE; row++) {
            for (uint8_t column = 0u;
                 column < AUTO_SNAKE_SEGMENT_SIZE; column++) {
                assert(!display_pixels[pixel_y + row][pixel_x + column]);
            }
        }
    }
}

static void test_cut_preserves_existing_food(void) {
    auto_snake_game_t game;
    auto_snake_cell_t current = 0u;
    auto_snake_cell_t food = 511u;

    memset(&game, 0, sizeof(game));
    game.random_state = 0x24681357u;
    game.move_interval_ms = AUTO_SNAKE_INITIAL_MOVE_MS;
    game.saved_move_interval_ms = AUTO_SNAKE_INITIAL_MOVE_MS;
    game.snake.length = 20u;
    game.snake.head_index = 19u;
    for (uint8_t index = 0u; index < game.snake.length; index++) {
        game.snake.body[index] = current;
        current = auto_snake_hamiltonian_next(current);
    }
    while (auto_snake_contains(&game.snake, food)) {
        food--;
    }
    game.food = food;
    game.food_available = true;
    auto_snake_cut_body(&game);
    assert(game.snake.length == AUTO_SNAKE_KEEP_LENGTH);
    assert(game.food_available);
    assert(game.food == food);
    assert(!auto_snake_detached_contains(
        &game.detached, game.food));
}

static void test_exit_to_menu(void) {
    auto_snake_game_t game;

    for (uint8_t state = AUTO_SNAKE_STATE_PLAYING;
         state <= AUTO_SNAKE_STATE_CUT_BODY; state++) {
        auto_snake_game_init(&game, 1u);
        game.state = (auto_snake_state_t)state;
        auto_snake_game_update(&game, GAME_INPUT_MODE, 1u);
        assert(game.return_to_menu);
    }
}

static void test_long_simulation(void) {
    uint32_t total_bites = 0u;
    uint32_t total_detached_cycles = 0u;
    uint32_t total_seek_steps = 0u;
    uint32_t total_fallbacks = 0u;

    for (uint32_t seed = 1u; seed <= 20u; seed++) {
        auto_snake_game_t game;
        uint16_t previous_food_count;
        uint16_t steps_since_food = 0u;

        auto_snake_game_init(&game, seed * 0x1020305u);
        previous_food_count = game.food_count;
        for (uint32_t tick = 0u; tick < 100000u; tick++) {
            auto_snake_state_t old_state = game.state;
            uint8_t old_recovery_steps = game.recovery_steps;
            bool detached_was_active = game.detached.active;
            auto_snake_cell_t old_head =
                game.snake.body[game.snake.head_index];
            auto_snake_cell_t old_tail =
                game.snake.body[auto_snake_body_index(
                    &game.snake, (uint8_t)(game.snake.length - 1u))];
            uint16_t old_head_order =
                auto_snake_hamiltonian_index(old_head);
            uint16_t old_tail_order =
                auto_snake_hamiltonian_index(old_tail);
            auto_snake_cell_t new_head;

            if (game.state == AUTO_SNAKE_STATE_BITE_ANIMATION) {
                auto_snake_game_update(
                    &game, 0u, AUTO_SNAKE_BITE_FRAME_MS);
            } else if (game.state == AUTO_SNAKE_STATE_CUT_BODY) {
                auto_snake_game_update(&game, 0u, 0u);
            } else {
                auto_snake_game_update(
                    &game, 0u, game.move_interval_ms);
            }
            new_head = game.snake.body[game.snake.head_index];
            if (detached_was_active && !game.detached.active) {
                total_detached_cycles++;
            }
            if ((old_recovery_steps == 1u) &&
                (game.recovery_steps == 0u)) {
                uint16_t head_order =
                    auto_snake_hamiltonian_index(new_head);

                for (uint8_t offset = 1u;
                     offset < game.snake.length; offset++) {
                    assert(auto_snake_hamiltonian_index(
                               auto_snake_body_at_offset(
                                   &game.snake, offset)) ==
                           ((head_order + 512u - offset) &
                            AUTO_SNAKE_CELL_MASK));
                }
            }
            if ((old_state == AUTO_SNAKE_STATE_SEEK_BITE) &&
                (game.state == AUTO_SNAKE_STATE_BITE_ANIMATION)) {
                bool bite_is_removable = false;

                total_bites++;
                total_seek_steps += game.bite_seek_steps;
                assert(auto_snake_manhattan_distance(
                    new_head, game.bite_cell) == 1u);
                for (uint8_t offset = AUTO_SNAKE_BITE_OFFSET;
                     offset < game.snake.length; offset++) {
                    if (auto_snake_body_at_offset(
                            &game.snake, offset) ==
                        game.bite_cell) {
                        bite_is_removable = true;
                        break;
                    }
                }
                assert(bite_is_removable);
            }
            assert(auto_snake_cell_x(new_head) < AUTO_SNAKE_GRID_WIDTH);
            assert(auto_snake_cell_y(new_head) < AUTO_SNAKE_GRID_HEIGHT);
            assert((old_head == new_head) ||
                   (auto_snake_manhattan_distance(
                       old_head, new_head) == 1u));
            assert(game.snake.length <= AUTO_SNAKE_MAX_LENGTH);
            assert(!auto_snake_detached_contains(
                &game.detached, new_head));
            if (game.food_available) {
                assert(!auto_snake_detached_contains(
                    &game.detached, game.food));
            }
            for (uint8_t offset = 1u;
                 offset < game.snake.length; offset++) {
                if (game.snake.body[
                        auto_snake_body_index(&game.snake, offset)] ==
                    new_head) {
                    fprintf(stderr,
                            "collision seed=%lu tick=%lu state=%u "
                            "recovery=%u length=%u offset=%u\n",
                            (unsigned long)seed, (unsigned long)tick,
                            (unsigned)old_state,
                            (unsigned)old_recovery_steps,
                            game.snake.length, offset);
                }
                assert(game.snake.body[
                           auto_snake_body_index(&game.snake, offset)] !=
                       new_head);
            }
            if ((old_head != new_head) &&
                (old_state == AUTO_SNAKE_STATE_PLAYING) &&
                (old_recovery_steps == 0u) &&
                (game.bite_count == 0u)) {
                assert(auto_snake_cycle_distance(
                           old_head_order,
                           auto_snake_hamiltonian_index(new_head)) > 0u);
                if (!(auto_snake_cycle_distance(
                          old_head_order,
                          auto_snake_hamiltonian_index(new_head)) +
                      AUTO_SNAKE_SAFETY_CELLS <=
                      auto_snake_cycle_distance(
                          old_head_order, old_tail_order))) {
                    fprintf(stderr,
                            "cycle invariant seed=%lu tick=%lu "
                            "length=%u bites=%u head=%u new=%u tail=%u\n",
                            (unsigned long)seed, (unsigned long)tick,
                            game.snake.length, game.bite_count,
                            old_head, new_head, old_tail);
                }
                assert(auto_snake_cycle_distance(
                           old_head_order,
                           auto_snake_hamiltonian_index(new_head)) +
                           AUTO_SNAKE_SAFETY_CELLS <=
                       auto_snake_cycle_distance(
                           old_head_order, old_tail_order));
            }
            if ((game.state != AUTO_SNAKE_STATE_PLAYING) ||
                game.detached.active ||
                (game.recovery_steps > 0u) ||
                game.bite_pending) {
                steps_since_food = 0u;
            } else if (game.food_count != previous_food_count) {
                previous_food_count = game.food_count;
                steps_since_food = 0u;
            } else {
                steps_since_food++;
                assert(steps_since_food <= 20000u);
            }
        }
        total_fallbacks += game.bite_seek_fallbacks;
    }
    assert(total_bites >= 100u);
    assert(total_detached_cycles >= 100u);
    printf("auto_snake: bites=%lu average_seek_steps=%lu "
           "fallbacks=%lu detached_cycles=%lu\n",
           (unsigned long)total_bites,
           (unsigned long)(total_seek_steps / total_bites),
           (unsigned long)total_fallbacks,
           (unsigned long)total_detached_cycles);
}

int main(void) {
    test_hamiltonian_cycle();
    test_safe_shortcut();
    test_initial_state_and_food();
    test_circular_wrap_and_bite();
    test_cut_preserves_existing_food();
    test_exit_to_menu();
    test_long_simulation();
    puts("auto_snake: all tests passed");
    return 0;
}
