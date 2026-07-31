#include "auto_snake_game.h"

#include <stddef.h>
#include <string.h>

#include "app.h"
#include "game_hal.h"

#define AUTO_SNAKE_INITIAL_LENGTH    (5u)
#define AUTO_SNAKE_INITIAL_MOVE_MS   (90u)
#define AUTO_SNAKE_MINIMUM_MOVE_MS   (40u)
#define AUTO_SNAKE_SPEED_STEP_MS     (2u)
#define AUTO_SNAKE_CELL_MASK         (0x01ffu)
#define AUTO_SNAKE_NO_CELL           (0xffffu)
#define AUTO_SNAKE_SAFETY_CELLS      (4u)
#define AUTO_SNAKE_BITE_MOVE_MS      (80u)
#define AUTO_SNAKE_BITE_FRAME_MS     (80u)
#define AUTO_SNAKE_BITE_FRAME_COUNT  (4u)
#define AUTO_SNAKE_BITE_SEEK_MAX_STEPS (1024u)
#define AUTO_SNAKE_BITE_FALLBACK_STEPS (512u)
#define AUTO_SNAKE_DETACHED_BLINK_MS (250u)
#define AUTO_SNAKE_DETACHED_DURATION_MS (5000u)
#define AUTO_SNAKE_DETACHED_REMOVE_MS (80u)

typedef struct {
    auto_snake_cell_t cell;
    uint16_t food_distance;
    uint16_t jump_distance;
    uint8_t free_neighbors;
    bool is_food;
    bool is_default;
    bool valid;
} auto_snake_candidate_t;

/*
 * SEEK_BITE is the only mode that uses BFS. Each queue entry stores a
 * 9-bit cell and the first direction taken from the head in bits 9..10.
 * The workspace is shared by every run of AUTO SNAKE and is never used by
 * normal food seeking.
 */
static uint16_t auto_snake_bite_queue[512];
static uint8_t auto_snake_bite_visited[64];

static auto_snake_cell_t auto_snake_recovery_next(
    const auto_snake_t *snake,
    const auto_snake_detached_tail_t *detached);
static void auto_snake_begin_bite_animation(
    auto_snake_game_t *game, auto_snake_cell_t bite_cell);

static uint8_t auto_snake_cell_x(auto_snake_cell_t cell) {
    return (uint8_t)(cell & 0x1fu);
}

static uint8_t auto_snake_cell_y(auto_snake_cell_t cell) {
    return (uint8_t)(cell >> 5);
}

static auto_snake_cell_t auto_snake_make_cell(uint8_t x, uint8_t y) {
    return (auto_snake_cell_t)(((uint16_t)y << 5) | x);
}

static auto_snake_cell_t auto_snake_hamiltonian_next(
    auto_snake_cell_t current) {
    uint8_t x = auto_snake_cell_x(current);
    uint8_t y = auto_snake_cell_y(current);

    if (y == 0u) {
        return (x < (AUTO_SNAKE_GRID_WIDTH - 1u)) ?
            auto_snake_make_cell((uint8_t)(x + 1u), 0u) :
            auto_snake_make_cell(x, 1u);
    }
    if (x == 0u) {
        return (y > 1u) ?
            auto_snake_make_cell(0u, (uint8_t)(y - 1u)) :
            auto_snake_make_cell(0u, 0u);
    }
    if ((y & 1u) != 0u) {
        if (x > 1u) {
            return auto_snake_make_cell((uint8_t)(x - 1u), y);
        }
        return (y == (AUTO_SNAKE_GRID_HEIGHT - 1u)) ?
            auto_snake_make_cell(0u, y) :
            auto_snake_make_cell(x, (uint8_t)(y + 1u));
    }
    return (x < (AUTO_SNAKE_GRID_WIDTH - 1u)) ?
        auto_snake_make_cell((uint8_t)(x + 1u), y) :
        auto_snake_make_cell(x, (uint8_t)(y + 1u));
}

static uint16_t auto_snake_hamiltonian_index(
    auto_snake_cell_t cell) {
    uint8_t x = auto_snake_cell_x(cell);
    uint8_t y = auto_snake_cell_y(cell);
    uint16_t row_start;

    if (y == 0u) {
        return x;
    }
    if (x == 0u) {
        return (uint16_t)(512u - y);
    }
    row_start = (uint16_t)(32u + (uint16_t)(y - 1u) * 31u);
    return ((y & 1u) != 0u) ?
        (uint16_t)(row_start + 31u - x) :
        (uint16_t)(row_start + x - 1u);
}

static uint16_t auto_snake_cycle_distance(uint16_t from, uint16_t to) {
    return (uint16_t)((to - from) & AUTO_SNAKE_CELL_MASK);
}

static uint32_t auto_snake_random(auto_snake_game_t *game) {
    uint32_t value = game->random_state;

    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    game->random_state = (value == 0u) ? 0x534e414bu : value;
    return game->random_state;
}

static uint8_t auto_snake_body_index(const auto_snake_t *snake,
                                     uint8_t offset_from_head) {
    uint8_t offset = (uint8_t)(offset_from_head %
                               AUTO_SNAKE_MAX_LENGTH);

    return (snake->head_index >= offset) ?
        (uint8_t)(snake->head_index - offset) :
        (uint8_t)(snake->head_index + AUTO_SNAKE_MAX_LENGTH - offset);
}

static auto_snake_cell_t auto_snake_body_at_offset(
    const auto_snake_t *snake, uint8_t offset_from_head) {
    return snake->body[
        auto_snake_body_index(snake, offset_from_head)];
}

static bool auto_snake_contains(const auto_snake_t *snake,
                                auto_snake_cell_t cell) {
    for (uint8_t offset = 0u; offset < snake->length; offset++) {
        if (snake->body[auto_snake_body_index(snake, offset)] == cell) {
            return true;
        }
    }
    return false;
}

static bool auto_snake_detached_contains(
    const auto_snake_detached_tail_t *detached,
    auto_snake_cell_t cell) {
    if ((detached == NULL) || !detached->active) {
        return false;
    }
    for (uint8_t index = 0u; index < detached->remaining; index++) {
        if (detached->cells[index] == cell) {
            return true;
        }
    }
    return false;
}

static bool auto_snake_cell_blocked(
    const auto_snake_t *snake,
    const auto_snake_detached_tail_t *detached,
    auto_snake_cell_t cell) {
    return auto_snake_contains(snake, cell) ||
           auto_snake_detached_contains(detached, cell);
}

static uint8_t auto_snake_unsigned_difference(uint8_t first,
                                              uint8_t second) {
    return (first > second) ?
        (uint8_t)(first - second) : (uint8_t)(second - first);
}

static uint16_t auto_snake_manhattan_distance(auto_snake_cell_t first,
                                              auto_snake_cell_t second) {
    return (uint16_t)(
        auto_snake_unsigned_difference(auto_snake_cell_x(first),
                                       auto_snake_cell_x(second)) +
        auto_snake_unsigned_difference(auto_snake_cell_y(first),
                                       auto_snake_cell_y(second)));
}

static bool auto_snake_is_safe_cycle_shortcut(
    const auto_snake_t *snake, auto_snake_cell_t candidate,
    bool tail_will_stay, uint16_t *jump_distance_out) {
    auto_snake_cell_t head = snake->body[snake->head_index];
    auto_snake_cell_t tail =
        snake->body[auto_snake_body_index(
            snake, (uint8_t)(snake->length - 1u))];
    uint16_t head_order = auto_snake_hamiltonian_index(head);
    uint16_t candidate_order = auto_snake_hamiltonian_index(candidate);
    uint16_t tail_order = auto_snake_hamiltonian_index(tail);
    uint16_t jump_distance =
        auto_snake_cycle_distance(head_order, candidate_order);
    uint16_t head_to_tail_distance =
        auto_snake_cycle_distance(head_order, tail_order);
    uint16_t required_safety = (uint16_t)(
        AUTO_SNAKE_SAFETY_CELLS + (tail_will_stay ? 1u : 0u));

    if (jump_distance_out != NULL) {
        *jump_distance_out = jump_distance;
    }
    return (jump_distance > 0u) &&
           ((uint16_t)(jump_distance + required_safety) <=
            head_to_tail_distance);
}

static uint8_t auto_snake_count_free_neighbors(
    const auto_snake_t *snake,
    const auto_snake_detached_tail_t *detached,
    auto_snake_cell_t cell) {
    uint8_t x = auto_snake_cell_x(cell);
    uint8_t y = auto_snake_cell_y(cell);
    uint8_t count = 0u;

    if ((y > 0u) &&
        !auto_snake_cell_blocked(snake, detached, auto_snake_make_cell(
            x, (uint8_t)(y - 1u)))) {
        count++;
    }
    if ((y + 1u < AUTO_SNAKE_GRID_HEIGHT) &&
        !auto_snake_cell_blocked(snake, detached, auto_snake_make_cell(
            x, (uint8_t)(y + 1u)))) {
        count++;
    }
    if ((x > 0u) &&
        !auto_snake_cell_blocked(snake, detached, auto_snake_make_cell(
            (uint8_t)(x - 1u), y))) {
        count++;
    }
    if ((x + 1u < AUTO_SNAKE_GRID_WIDTH) &&
        !auto_snake_cell_blocked(snake, detached, auto_snake_make_cell(
            (uint8_t)(x + 1u), y))) {
        count++;
    }
    return count;
}

static bool auto_snake_candidate_is_better(
    const auto_snake_candidate_t *candidate,
    const auto_snake_candidate_t *best) {
    if (!best->valid) {
        return true;
    }
    if (candidate->is_food != best->is_food) {
        return candidate->is_food;
    }
    if (candidate->food_distance != best->food_distance) {
        return candidate->food_distance < best->food_distance;
    }
    if (candidate->free_neighbors != best->free_neighbors) {
        return candidate->free_neighbors > best->free_neighbors;
    }
    if (candidate->jump_distance != best->jump_distance) {
        return candidate->jump_distance < best->jump_distance;
    }
    return candidate->is_default && !best->is_default;
}

static auto_snake_cell_t auto_snake_choose_next(
    const auto_snake_t *snake,
    const auto_snake_detached_tail_t *detached,
    auto_snake_cell_t food) {
    static const int8_t direction_x[4] = {0, 0, -1, 1};
    static const int8_t direction_y[4] = {-1, 1, 0, 0};
    auto_snake_cell_t head = snake->body[snake->head_index];
    auto_snake_cell_t default_cell = auto_snake_hamiltonian_next(head);
    auto_snake_candidate_t best = {0};
    uint16_t head_order = auto_snake_hamiltonian_index(head);
    uint16_t head_to_food_distance = auto_snake_cycle_distance(
        head_order, auto_snake_hamiltonian_index(food));
    uint8_t head_x = auto_snake_cell_x(head);
    uint8_t head_y = auto_snake_cell_y(head);

    for (uint8_t direction = 0u; direction < 4u; direction++) {
        int16_t candidate_x = (int16_t)head_x + direction_x[direction];
        int16_t candidate_y = (int16_t)head_y + direction_y[direction];
        auto_snake_candidate_t candidate = {0};

        if ((candidate_x < 0) ||
            (candidate_x >= (int16_t)AUTO_SNAKE_GRID_WIDTH) ||
            (candidate_y < 0) ||
            (candidate_y >= (int16_t)AUTO_SNAKE_GRID_HEIGHT)) {
            continue;
        }
        candidate.cell = auto_snake_make_cell(
            (uint8_t)candidate_x, (uint8_t)candidate_y);
        if (auto_snake_cell_blocked(snake, detached, candidate.cell) ||
            !auto_snake_is_safe_cycle_shortcut(
                snake, candidate.cell, candidate.cell == food,
                &candidate.jump_distance)) {
            continue;
        }
        candidate.food_distance =
            auto_snake_manhattan_distance(candidate.cell, food);
        /*
         * Never jump over the food in Hamiltonian order. Without this guard,
         * Manhattan shortcuts can repeatedly pass a nearby food from its
         * unsafe side and make it look as though the snake refuses to eat.
         */
        if (candidate.jump_distance > head_to_food_distance) {
            continue;
        }
        candidate.free_neighbors =
            auto_snake_count_free_neighbors(
                snake, detached, candidate.cell);
        if (candidate.free_neighbors == 0u) {
            continue;
        }
        candidate.is_food = candidate.cell == food;
        candidate.is_default = candidate.cell == default_cell;
        candidate.valid = true;
        if (auto_snake_candidate_is_better(&candidate, &best)) {
            best = candidate;
        }
    }
    if (best.valid) {
        return best.cell;
    }
    return auto_snake_cell_blocked(snake, detached, default_cell) ?
        auto_snake_recovery_next(snake, detached) : default_cell;
}

static bool auto_snake_bite_visited_get(auto_snake_cell_t cell) {
    return (auto_snake_bite_visited[cell >> 3] &
            (uint8_t)(1u << (cell & 7u))) != 0u;
}

static void auto_snake_bite_visited_set(auto_snake_cell_t cell) {
    auto_snake_bite_visited[cell >> 3] |=
        (uint8_t)(1u << (cell & 7u));
}

static bool auto_snake_neighbor_in_direction(
    auto_snake_cell_t cell, uint8_t direction,
    auto_snake_cell_t *neighbor) {
    uint8_t x = auto_snake_cell_x(cell);
    uint8_t y = auto_snake_cell_y(cell);

    if (direction == 0u) {
        if (y == 0u) {
            return false;
        }
        *neighbor = auto_snake_make_cell(x, (uint8_t)(y - 1u));
    } else if (direction == 1u) {
        if (y + 1u >= AUTO_SNAKE_GRID_HEIGHT) {
            return false;
        }
        *neighbor = auto_snake_make_cell(x, (uint8_t)(y + 1u));
    } else if (direction == 2u) {
        if (x == 0u) {
            return false;
        }
        *neighbor = auto_snake_make_cell((uint8_t)(x - 1u), y);
    } else {
        if (x + 1u >= AUTO_SNAKE_GRID_WIDTH) {
            return false;
        }
        *neighbor = auto_snake_make_cell((uint8_t)(x + 1u), y);
    }
    return true;
}

static bool auto_snake_bite_position_has_escape(
    const auto_snake_t *snake, auto_snake_cell_t candidate,
    auto_snake_cell_t bite_cell) {
    auto_snake_cell_t recovery_cell = candidate;

    if (auto_snake_manhattan_distance(candidate, bite_cell) != 1u) {
        return false;
    }
    /*
     * After the cut, ten default Hamiltonian moves must be available so the
     * retained head section can restore the normal cycle invariant.
     */
    for (uint8_t step = 0u;
         step < AUTO_SNAKE_KEEP_LENGTH; step++) {
        recovery_cell =
            auto_snake_hamiltonian_next(recovery_cell);
        for (uint8_t offset = 0u;
             offset < AUTO_SNAKE_KEEP_LENGTH - 1u; offset++) {
            if (auto_snake_body_at_offset(
                    snake, offset) == recovery_cell) {
                return false;
            }
        }
    }
    for (uint8_t direction = 0u; direction < 4u; direction++) {
        auto_snake_cell_t escape_cell;
        bool retained_cell = false;

        if (!auto_snake_neighbor_in_direction(
                candidate, direction, &escape_cell)) {
            continue;
        }
        for (uint8_t offset = 0u;
             offset < AUTO_SNAKE_KEEP_LENGTH - 1u; offset++) {
            if (auto_snake_body_at_offset(
                    snake, offset) == escape_cell) {
                retained_cell = true;
                break;
            }
        }
        if (retained_cell) {
            continue;
        }
        for (uint8_t escape_direction = 0u;
             escape_direction < 4u; escape_direction++) {
            auto_snake_cell_t onward_cell;
            bool onward_is_retained = false;

            if (!auto_snake_neighbor_in_direction(
                    escape_cell, escape_direction, &onward_cell) ||
                (onward_cell == candidate)) {
                continue;
            }
            for (uint8_t offset = 0u;
                 offset < AUTO_SNAKE_KEEP_LENGTH - 1u; offset++) {
                if (auto_snake_body_at_offset(
                        snake, offset) == onward_cell) {
                    onward_is_retained = true;
                    break;
                }
            }
            if (!onward_is_retained) {
                return true;
            }
        }
    }
    return false;
}

static auto_snake_cell_t auto_snake_bite_bfs_next(
    const auto_snake_t *snake,
    const auto_snake_detached_tail_t *detached,
    auto_snake_cell_t bite_cell,
    bool *path_found) {
    auto_snake_cell_t head = snake->body[snake->head_index];
    uint16_t read_index = 0u;
    uint16_t write_index = 0u;

    memset(auto_snake_bite_visited, 0, sizeof(auto_snake_bite_visited));
    auto_snake_bite_visited_set(head);
    for (uint8_t direction = 0u; direction < 4u; direction++) {
        auto_snake_cell_t neighbor;

        if (!auto_snake_neighbor_in_direction(
                head, direction, &neighbor) ||
            auto_snake_cell_blocked(snake, detached, neighbor)) {
            continue;
        }
        if (auto_snake_bite_position_has_escape(
                snake, neighbor, bite_cell)) {
            *path_found = true;
            return neighbor;
        }
        auto_snake_bite_visited_set(neighbor);
        auto_snake_bite_queue[write_index++] =
            (uint16_t)(neighbor | ((uint16_t)direction << 9));
    }

    while (read_index < write_index) {
        uint16_t entry = auto_snake_bite_queue[read_index++];
        auto_snake_cell_t cell =
            (auto_snake_cell_t)(entry & AUTO_SNAKE_CELL_MASK);
        uint8_t first_direction = (uint8_t)((entry >> 9) & 3u);

        for (uint8_t direction = 0u; direction < 4u; direction++) {
            auto_snake_cell_t neighbor;

            if (!auto_snake_neighbor_in_direction(
                    cell, direction, &neighbor) ||
                auto_snake_bite_visited_get(neighbor) ||
                auto_snake_cell_blocked(snake, detached, neighbor)) {
                continue;
            }
            if (auto_snake_bite_position_has_escape(
                    snake, neighbor, bite_cell)) {
                auto_snake_cell_t next = head;

                (void)auto_snake_neighbor_in_direction(
                    head, first_direction, &next);
                *path_found = true;
                return next;
            }
            auto_snake_bite_visited_set(neighbor);
            auto_snake_bite_queue[write_index++] =
                (uint16_t)(neighbor |
                           ((uint16_t)first_direction << 9));
        }
    }
    *path_found = false;
    return auto_snake_hamiltonian_next(head);
}

static void auto_snake_advance_without_growing(
    auto_snake_game_t *game, auto_snake_cell_t next) {
    auto_snake_t *snake = &game->snake;

    game->previous_head = snake->body[snake->head_index];
    game->previous_head_valid = true;
    game->erased_tail = auto_snake_body_at_offset(
        snake, (uint8_t)(snake->length - 1u));
    game->erased_tail_valid = true;
    snake->head_index = (snake->head_index ==
                         (AUTO_SNAKE_MAX_LENGTH - 1u)) ?
        0u : (uint8_t)(snake->head_index + 1u);
    snake->body[snake->head_index] = next;
}

static void auto_snake_place_food(auto_snake_game_t *game) {
    auto_snake_cell_t food;

    do {
        food = (auto_snake_cell_t)(
            auto_snake_random(game) & AUTO_SNAKE_CELL_MASK);
    } while (auto_snake_cell_blocked(
        &game->snake, &game->detached, food));
    game->food = food;
    game->food_available = true;
    game->food_dirty = true;
}

static void auto_snake_place_food_if_empty(auto_snake_game_t *game) {
    if (!game->food_available) {
        auto_snake_place_food(game);
    }
}

static auto_snake_cell_t auto_snake_recovery_next(
    const auto_snake_t *snake,
    const auto_snake_detached_tail_t *detached) {
    auto_snake_cell_t head = snake->body[snake->head_index];
    auto_snake_cell_t next = auto_snake_hamiltonian_next(head);
    auto_snake_cell_t best = next;
    uint8_t best_free_neighbors = 0u;

    if (!auto_snake_cell_blocked(snake, detached, next)) {
        return next;
    }
    for (uint8_t direction = 0u; direction < 4u; direction++) {
        auto_snake_cell_t candidate;
        uint8_t free_neighbors;

        if (!auto_snake_neighbor_in_direction(
                head, direction, &candidate) ||
            auto_snake_cell_blocked(snake, detached, candidate)) {
            continue;
        }
        free_neighbors =
            auto_snake_count_free_neighbors(
                snake, detached, candidate);
        if ((best_free_neighbors == 0u) ||
            (free_neighbors > best_free_neighbors)) {
            best = candidate;
            best_free_neighbors = free_neighbors;
        }
    }
    return best;
}

static auto_snake_cell_t auto_snake_recovery_bfs_next(
    const auto_snake_t *snake,
    const auto_snake_detached_tail_t *detached) {
    auto_snake_cell_t head = snake->body[snake->head_index];
    uint16_t read_index = 0u;
    uint16_t write_index = 0u;

    memset(auto_snake_bite_visited, 0, sizeof(auto_snake_bite_visited));
    auto_snake_bite_visited_set(head);
    for (uint8_t direction = 0u; direction < 4u; direction++) {
        auto_snake_cell_t neighbor;

        if (!auto_snake_neighbor_in_direction(
                head, direction, &neighbor) ||
            auto_snake_cell_blocked(snake, detached, neighbor)) {
            continue;
        }
        if (auto_snake_count_free_neighbors(
                snake, detached, neighbor) >= 2u) {
            return neighbor;
        }
        auto_snake_bite_visited_set(neighbor);
        auto_snake_bite_queue[write_index++] =
            (uint16_t)(neighbor | ((uint16_t)direction << 9));
    }
    while (read_index < write_index) {
        uint16_t entry = auto_snake_bite_queue[read_index++];
        auto_snake_cell_t cell =
            (auto_snake_cell_t)(entry & AUTO_SNAKE_CELL_MASK);
        uint8_t first_direction = (uint8_t)((entry >> 9) & 3u);

        for (uint8_t direction = 0u; direction < 4u; direction++) {
            auto_snake_cell_t neighbor;

            if (!auto_snake_neighbor_in_direction(
                    cell, direction, &neighbor) ||
                auto_snake_bite_visited_get(neighbor) ||
                auto_snake_cell_blocked(snake, detached, neighbor)) {
                continue;
            }
            if (auto_snake_count_free_neighbors(
                    snake, detached, neighbor) >= 2u) {
                auto_snake_cell_t next = head;

                (void)auto_snake_neighbor_in_direction(
                    head, first_direction, &next);
                return next;
            }
            auto_snake_bite_visited_set(neighbor);
            auto_snake_bite_queue[write_index++] =
                (uint16_t)(neighbor |
                           ((uint16_t)first_direction << 9));
        }
    }
    return auto_snake_recovery_next(snake, detached);
}

static auto_snake_cell_t auto_snake_food_bfs_next(
    const auto_snake_t *snake,
    const auto_snake_detached_tail_t *detached,
    auto_snake_cell_t food, bool *path_found) {
    auto_snake_cell_t head = snake->body[snake->head_index];
    uint16_t read_index = 0u;
    uint16_t write_index = 0u;

    memset(auto_snake_bite_visited, 0, sizeof(auto_snake_bite_visited));
    auto_snake_bite_visited_set(head);
    for (uint8_t direction = 0u; direction < 4u; direction++) {
        auto_snake_cell_t neighbor;

        if (!auto_snake_neighbor_in_direction(
                head, direction, &neighbor) ||
            auto_snake_cell_blocked(snake, detached, neighbor)) {
            continue;
        }
        if (neighbor == food) {
            *path_found = true;
            return neighbor;
        }
        auto_snake_bite_visited_set(neighbor);
        auto_snake_bite_queue[write_index++] =
            (uint16_t)(neighbor | ((uint16_t)direction << 9));
    }
    while (read_index < write_index) {
        uint16_t entry = auto_snake_bite_queue[read_index++];
        auto_snake_cell_t cell =
            (auto_snake_cell_t)(entry & AUTO_SNAKE_CELL_MASK);
        uint8_t first_direction = (uint8_t)((entry >> 9) & 3u);

        for (uint8_t direction = 0u; direction < 4u; direction++) {
            auto_snake_cell_t neighbor;

            if (!auto_snake_neighbor_in_direction(
                    cell, direction, &neighbor) ||
                auto_snake_bite_visited_get(neighbor) ||
                auto_snake_cell_blocked(
                    snake, detached, neighbor)) {
                continue;
            }
            if (neighbor == food) {
                auto_snake_cell_t next = head;

                (void)auto_snake_neighbor_in_direction(
                    head, first_direction, &next);
                *path_found = true;
                return next;
            }
            auto_snake_bite_visited_set(neighbor);
            auto_snake_bite_queue[write_index++] =
                (uint16_t)(neighbor |
                           ((uint16_t)first_direction << 9));
        }
    }
    *path_found = false;
    return head;
}

static void auto_snake_move_playing(auto_snake_game_t *game) {
    auto_snake_t *snake = &game->snake;
    auto_snake_cell_t old_head = snake->body[snake->head_index];
    auto_snake_cell_t next =
        auto_snake_choose_next(
            snake, &game->detached, game->food);
    bool food_path_found = false;

    if (game->resume_old_food && game->food_available &&
        (game->recovery_steps == 0u)) {
        auto_snake_cell_t food_next = auto_snake_food_bfs_next(
            snake, &game->detached, game->food,
            &food_path_found);

        if (food_path_found) {
            next = food_next;
        }
    }
    bool ate = game->food_available && (next == game->food);

    if (game->recovery_steps > 0u) {
        auto_snake_cell_t recovery_next;
        auto_snake_cell_t default_next =
            auto_snake_hamiltonian_next(
                snake->body[snake->head_index]);
        bool followed_default = false;

        if (!auto_snake_cell_blocked(
                snake, &game->detached, default_next)) {
            recovery_next = default_next;
            followed_default = true;
        } else {
            recovery_next = auto_snake_recovery_bfs_next(
                snake, &game->detached);
        }
        if (auto_snake_cell_blocked(
                snake, &game->detached, recovery_next)) {
            return;
        }
        auto_snake_advance_without_growing(game, recovery_next);
        game->recovery_steps = followed_default ?
            (uint8_t)(game->recovery_steps - 1u) :
            AUTO_SNAKE_KEEP_LENGTH;
        if (game->recovery_steps == 0u) {
            auto_snake_place_food_if_empty(game);
        }
        return;
    }
    if (auto_snake_cell_blocked(
            snake, &game->detached, next)) {
        if (!game->detached.active &&
            (snake->length > AUTO_SNAKE_KEEP_LENGTH)) {
            for (uint8_t direction = 0u;
                 direction < 4u; direction++) {
                auto_snake_cell_t neighbor;

                if (!auto_snake_neighbor_in_direction(
                        old_head, direction, &neighbor)) {
                    continue;
                }
                for (uint8_t offset = AUTO_SNAKE_BITE_OFFSET;
                     offset < snake->length; offset++) {
                    if (auto_snake_body_at_offset(
                            snake, offset) == neighbor) {
                        game->saved_move_interval_ms =
                            game->move_interval_ms;
                        game->bite_seek_fallbacks++;
                        auto_snake_begin_bite_animation(
                            game, neighbor);
                        return;
                    }
                }
            }
        }
        return;
    }

    game->previous_head = old_head;
    game->previous_head_valid = true;
    game->erased_tail_valid = false;
    snake->head_index = (snake->head_index ==
                         (AUTO_SNAKE_MAX_LENGTH - 1u)) ?
        0u : (uint8_t)(snake->head_index + 1u);
    snake->body[snake->head_index] = next;

    if (ate) {
        game->food_available = false;
        game->resume_old_food = false;
        if (snake->length < AUTO_SNAKE_MAX_LENGTH) {
            snake->length++;
        }
        game->food_count++;
        if (game->move_interval_ms > AUTO_SNAKE_MINIMUM_MOVE_MS) {
            uint16_t next_interval = (uint16_t)(
                game->move_interval_ms - AUTO_SNAKE_SPEED_STEP_MS);

            game->move_interval_ms =
                (next_interval < AUTO_SNAKE_MINIMUM_MOVE_MS) ?
                AUTO_SNAKE_MINIMUM_MOVE_MS : next_interval;
        }
        if (snake->length >= AUTO_SNAKE_MAX_LENGTH) {
            /*
             * Generate the next target before self-bite starts. It remains
             * unchanged throughout SEEK/BITE/CUT and becomes the target that
             * the shortened snake continues to chase afterwards.
             */
            auto_snake_place_food(game);
            game->resume_old_food = true;
            if (game->detached.active) {
                game->bite_pending = true;
                game->redraw_all = true;
            } else {
                game->state = AUTO_SNAKE_STATE_SEEK_BITE;
                game->saved_move_interval_ms =
                    game->move_interval_ms;
                game->move_interval_ms =
                    AUTO_SNAKE_BITE_MOVE_MS;
                game->bite_seek_steps = 0u;
                game->bite_fallback_steps = 0u;
                game->bite_cell = auto_snake_body_at_offset(
                    snake, AUTO_SNAKE_BITE_OFFSET);
                game->redraw_all = true;
                game->erased_tail_valid = false;
                game->previous_head_valid = false;
            }
        } else {
            auto_snake_place_food(game);
        }
        return;
    }

    game->erased_tail =
        snake->body[auto_snake_body_index(snake, snake->length)];
    game->erased_tail_valid = true;
}

static void auto_snake_begin_bite_animation(
    auto_snake_game_t *game, auto_snake_cell_t bite_cell) {
    game->bite_cell = bite_cell;
    game->state = AUTO_SNAKE_STATE_BITE_ANIMATION;
    game->bite_frame = 0u;
    game->bite_frame_accumulator_ms = 0u;
    game->previous_head_valid = false;
}

static void auto_snake_seek_bite_step(auto_snake_game_t *game) {
    auto_snake_t *snake = &game->snake;
    auto_snake_cell_t head = snake->body[snake->head_index];
    auto_snake_cell_t next;
    bool path_found = false;

    game->bite_cell = auto_snake_body_at_offset(
        snake, AUTO_SNAKE_BITE_OFFSET);
    if (auto_snake_manhattan_distance(head, game->bite_cell) == 1u) {
        auto_snake_begin_bite_animation(game, game->bite_cell);
        return;
    }

    next = auto_snake_bite_bfs_next(
        snake, &game->detached, game->bite_cell, &path_found);
    if (!path_found) {
        next = auto_snake_recovery_bfs_next(
            snake, &game->detached);
        if (game->bite_fallback_steps > 0u) {
            game->bite_fallback_steps--;
        }
    }
    if (game->food_available && (next == game->food)) {
        auto_snake_cell_t alternative = head;
        uint8_t best_free_neighbors = 0u;

        for (uint8_t direction = 0u; direction < 4u; direction++) {
            auto_snake_cell_t candidate;
            uint8_t free_neighbors;

            if (!auto_snake_neighbor_in_direction(
                    head, direction, &candidate) ||
                (candidate == game->food) ||
                auto_snake_cell_blocked(
                    snake, &game->detached, candidate)) {
                continue;
            }
            free_neighbors = auto_snake_count_free_neighbors(
                snake, &game->detached, candidate);
            if ((alternative == head) ||
                (free_neighbors > best_free_neighbors)) {
                alternative = candidate;
                best_free_neighbors = free_neighbors;
            }
        }
        if (alternative == head) {
            return;
        }
        next = alternative;
    }
    if (auto_snake_cell_blocked(
            snake, &game->detached, next)) {
        next = auto_snake_recovery_next(
            snake, &game->detached);
    }
    if (auto_snake_cell_blocked(
            snake, &game->detached, next)) {
        /*
         * The moving target can rarely leave the head fully enclosed.
         * Bite an adjacent segment from the removable portion instead of
         * stepping through the body or truncating without animation.
         */
        for (uint8_t direction = 0u; direction < 4u; direction++) {
            auto_snake_cell_t neighbor;

            if (!auto_snake_neighbor_in_direction(
                    head, direction, &neighbor)) {
                continue;
            }
            for (uint8_t offset = AUTO_SNAKE_BITE_OFFSET;
                 offset < snake->length; offset++) {
                if (auto_snake_body_at_offset(snake, offset) ==
                    neighbor) {
                    game->bite_seek_fallbacks++;
                    auto_snake_begin_bite_animation(game, neighbor);
                    return;
                }
            }
        }
        game->bite_seek_fallbacks++;
        return;
    }
    {
        auto_snake_cell_t bite_cell_before_move = game->bite_cell;
        bool reached_safe_bite_position =
            auto_snake_bite_position_has_escape(
                snake, next, bite_cell_before_move);

        auto_snake_advance_without_growing(game, next);
        game->bite_seek_steps++;
        if (reached_safe_bite_position) {
            auto_snake_begin_bite_animation(
                game, bite_cell_before_move);
            return;
        }
    }
    game->bite_cell = auto_snake_body_at_offset(
        snake, AUTO_SNAKE_BITE_OFFSET);
    if (game->bite_seek_steps >= AUTO_SNAKE_BITE_SEEK_MAX_STEPS) {
        game->bite_seek_steps = 0u;
        game->bite_seek_fallbacks++;
        game->bite_fallback_steps = AUTO_SNAKE_BITE_FALLBACK_STEPS;
    }
}

static void auto_snake_cut_body(auto_snake_game_t *game) {
    auto_snake_detached_tail_t *detached = &game->detached;

    if (detached->active) {
        game->bite_pending = true;
        game->state = AUTO_SNAKE_STATE_SEEK_BITE;
        game->bite_seek_fallbacks++;
        return;
    }
    game->cut_previous_length = game->snake.length;
    detached->length = (uint8_t)(
        game->snake.length - AUTO_SNAKE_KEEP_LENGTH);
    detached->remaining = detached->length;
    for (uint8_t index = 0u; index < detached->length; index++) {
        detached->cells[index] = auto_snake_body_at_offset(
            &game->snake,
            (uint8_t)(AUTO_SNAKE_BITE_OFFSET + index));
    }
    detached->state = AUTO_SNAKE_DETACHED_BLINKING;
    detached->state_elapsed_ms = 0u;
    detached->update_accumulator_ms = 0u;
    detached->visible = true;
    detached->active = true;
    detached->dirty = true;
    detached->last_removed_valid = false;
    game->snake.length = AUTO_SNAKE_KEEP_LENGTH;
    game->bite_count++;
    game->move_interval_ms = game->saved_move_interval_ms;
    game->state = AUTO_SNAKE_STATE_PLAYING;
    game->recovery_steps = AUTO_SNAKE_KEEP_LENGTH;
    game->cut_redraw = true;
    game->previous_head_valid = false;
    game->erased_tail_valid = false;
}

static void auto_snake_update_detached(
    auto_snake_detached_tail_t *detached, uint32_t elapsed_ms) {
    if (!detached->active) {
        return;
    }
    detached->state_elapsed_ms += elapsed_ms;
    detached->update_accumulator_ms += elapsed_ms;
    if (detached->state == AUTO_SNAKE_DETACHED_BLINKING) {
        while (detached->update_accumulator_ms >=
               AUTO_SNAKE_DETACHED_BLINK_MS) {
            detached->update_accumulator_ms -=
                AUTO_SNAKE_DETACHED_BLINK_MS;
            detached->visible = !detached->visible;
            detached->dirty = true;
        }
        if (detached->state_elapsed_ms >=
            AUTO_SNAKE_DETACHED_DURATION_MS) {
            detached->state =
                AUTO_SNAKE_DETACHED_DISAPPEARING;
            detached->state_elapsed_ms = 0u;
            detached->update_accumulator_ms = 0u;
            detached->visible = true;
            detached->dirty = true;
        }
    } else {
        while ((detached->remaining > 0u) &&
               (detached->update_accumulator_ms >=
                AUTO_SNAKE_DETACHED_REMOVE_MS)) {
            detached->update_accumulator_ms -=
                AUTO_SNAKE_DETACHED_REMOVE_MS;
            detached->remaining--;
            detached->last_removed_cell =
                detached->cells[detached->remaining];
            detached->last_removed_valid = true;
            detached->dirty = true;
        }
        if (detached->remaining == 0u) {
            detached->state = AUTO_SNAKE_DETACHED_NONE;
            detached->active = false;
            detached->visible = false;
            detached->dirty = true;
        }
    }
}

static void auto_snake_draw_cell(auto_snake_cell_t cell, bool on) {
    game_display_fill_rect((uint8_t)(auto_snake_cell_x(cell) << 2),
                           (uint8_t)(auto_snake_cell_y(cell) << 2),
                           AUTO_SNAKE_SEGMENT_SIZE,
                           AUTO_SNAKE_SEGMENT_SIZE, on);
}

static void auto_snake_draw_blink_cell(auto_snake_cell_t cell) {
    uint8_t pixel_x = (uint8_t)(auto_snake_cell_x(cell) << 2);
    uint8_t pixel_y = (uint8_t)(auto_snake_cell_y(cell) << 2);

    game_display_draw_pixel(pixel_x, pixel_y, true);
    game_display_draw_pixel((uint8_t)(pixel_x + 2u),
                            (uint8_t)(pixel_y + 2u), true);
}

static void auto_snake_draw_head(auto_snake_cell_t cell) {
    uint8_t pixel_x = (uint8_t)(auto_snake_cell_x(cell) << 2);
    uint8_t pixel_y = (uint8_t)(auto_snake_cell_y(cell) << 2);

    game_display_fill_rect(pixel_x, pixel_y, AUTO_SNAKE_SEGMENT_SIZE,
                           AUTO_SNAKE_SEGMENT_SIZE, true);
    game_display_draw_pixel((uint8_t)(pixel_x + 2u), pixel_y, false);
}

static void auto_snake_draw_bite_frame(const auto_snake_game_t *game) {
    auto_snake_cell_t head =
        game->snake.body[game->snake.head_index];
    uint8_t head_x = (uint8_t)(auto_snake_cell_x(head) << 2);
    uint8_t head_y = (uint8_t)(auto_snake_cell_y(head) << 2);
    uint8_t bite_x = auto_snake_cell_x(game->bite_cell);
    uint8_t bite_y = auto_snake_cell_y(game->bite_cell);
    uint8_t frame = game->bite_frame;

    auto_snake_draw_cell(game->bite_cell, true);
    auto_snake_draw_head(head);
    if (frame == 1u) {
        if (bite_x > auto_snake_cell_x(head)) {
            game_display_draw_pixel((uint8_t)(head_x + 2u),
                                    (uint8_t)(head_y + 1u), false);
        } else if (bite_x < auto_snake_cell_x(head)) {
            game_display_draw_pixel(head_x,
                                    (uint8_t)(head_y + 1u), false);
        } else if (bite_y > auto_snake_cell_y(head)) {
            game_display_draw_pixel((uint8_t)(head_x + 1u),
                                    (uint8_t)(head_y + 2u), false);
        } else {
            game_display_draw_pixel((uint8_t)(head_x + 1u),
                                    head_y, false);
        }
    } else if (frame == 3u) {
        auto_snake_draw_cell(game->bite_cell, false);
    }
}

static void auto_snake_draw_food(auto_snake_cell_t cell) {
    uint8_t pixel_x = (uint8_t)(auto_snake_cell_x(cell) << 2);
    uint8_t pixel_y = (uint8_t)(auto_snake_cell_y(cell) << 2);

    game_display_draw_pixel((uint8_t)(pixel_x + 1u), pixel_y, true);
    game_display_fill_rect(pixel_x, (uint8_t)(pixel_y + 1u), 3u, 1u, true);
    game_display_draw_pixel((uint8_t)(pixel_x + 1u),
                            (uint8_t)(pixel_y + 2u), true);
}

void auto_snake_game_init(void *context, uint32_t seed) {
    auto_snake_game_t *game = (auto_snake_game_t *)context;
    auto_snake_cell_t cell = auto_snake_make_cell(0u, 0u);

    if (game == NULL) {
        return;
    }
    memset(game, 0, sizeof(*game));
    game->random_state = (seed == 0u) ? 0x534e414bu : seed;
    game->move_interval_ms = AUTO_SNAKE_INITIAL_MOVE_MS;
    game->erased_tail = AUTO_SNAKE_NO_CELL;
    for (uint8_t index = 0u; index < AUTO_SNAKE_INITIAL_LENGTH; index++) {
        game->snake.body[index] = cell;
        game->snake.head_index = index;
        cell = auto_snake_hamiltonian_next(cell);
    }
    game->snake.length = AUTO_SNAKE_INITIAL_LENGTH;
    auto_snake_place_food(game);
    game->redraw_all = true;
}

void auto_snake_game_deinit(void *context) {
    auto_snake_game_t *game = (auto_snake_game_t *)context;

    if (game != NULL) {
        memset(&game->detached, 0, sizeof(game->detached));
    }
}

void auto_snake_game_update(void *context, uint8_t input_state,
                            uint32_t elapsed_ms) {
    auto_snake_game_t *game = (auto_snake_game_t *)context;
    uint8_t pressed;
    uint8_t move_count = 0u;

    if (game == NULL) {
        return;
    }
    pressed = (uint8_t)(input_state & ~game->input_previous);
    game->input_previous = input_state;
    if ((pressed & GAME_INPUT_MODE) != 0u) {
        game->return_to_menu = true;
        return;
    }

    auto_snake_update_detached(&game->detached, elapsed_ms);
    if (game->bite_pending) {
        if (game->detached.active) {
            return;
        }
        game->bite_pending = false;
        game->state = AUTO_SNAKE_STATE_SEEK_BITE;
        game->saved_move_interval_ms = game->move_interval_ms;
        game->move_interval_ms = AUTO_SNAKE_BITE_MOVE_MS;
        game->bite_seek_steps = 0u;
        game->bite_fallback_steps = 0u;
        game->bite_cell = auto_snake_body_at_offset(
            &game->snake, AUTO_SNAKE_BITE_OFFSET);
        game->redraw_all = true;
    }

    if (game->state == AUTO_SNAKE_STATE_BITE_ANIMATION) {
        game->bite_frame_accumulator_ms =
            (uint16_t)(game->bite_frame_accumulator_ms + elapsed_ms);
        while (game->bite_frame_accumulator_ms >=
               AUTO_SNAKE_BITE_FRAME_MS) {
            game->bite_frame_accumulator_ms -=
                AUTO_SNAKE_BITE_FRAME_MS;
            game->bite_frame++;
            if (game->bite_frame >= AUTO_SNAKE_BITE_FRAME_COUNT) {
                game->state = AUTO_SNAKE_STATE_CUT_BODY;
                break;
            }
        }
        return;
    }
    if (game->state == AUTO_SNAKE_STATE_CUT_BODY) {
        auto_snake_cut_body(game);
        return;
    }

    game->move_accumulator_ms += elapsed_ms;
    while (game->move_accumulator_ms >= game->move_interval_ms) {
        game->move_accumulator_ms -= game->move_interval_ms;
        if (game->state == AUTO_SNAKE_STATE_PLAYING) {
            auto_snake_move_playing(game);
        } else {
            auto_snake_seek_bite_step(game);
        }
        move_count++;
        if (move_count > 1u) {
            game->redraw_all = true;
        }
        if ((game->state == AUTO_SNAKE_STATE_BITE_ANIMATION) ||
            (game->state == AUTO_SNAKE_STATE_CUT_BODY)) {
            break;
        }
    }
}

void auto_snake_game_draw(const void *context) {
    auto_snake_game_t *game = (auto_snake_game_t *)context;

    if (game == NULL) {
        return;
    }
    if (game->cut_redraw) {
        for (uint8_t offset = 1u;
             offset < game->snake.length; offset++) {
            auto_snake_draw_cell(
                auto_snake_body_at_offset(&game->snake, offset), true);
        }
        auto_snake_draw_head(
            game->snake.body[game->snake.head_index]);
        if (game->food_available) {
            auto_snake_draw_food(game->food);
        }
    } else if (game->redraw_all) {
        game_display_clear();
        for (uint8_t offset = 1u; offset < game->snake.length; offset++) {
            auto_snake_draw_cell(
                game->snake.body[
                    auto_snake_body_index(&game->snake, offset)], true);
        }
        auto_snake_draw_head(
            game->snake.body[game->snake.head_index]);
        if ((game->state == AUTO_SNAKE_STATE_PLAYING) &&
            game->food_available) {
            auto_snake_draw_food(game->food);
        }
    } else {
        if (game->erased_tail_valid) {
            auto_snake_draw_cell(game->erased_tail, false);
        }
        if (game->previous_head_valid) {
            auto_snake_draw_cell(game->previous_head, true);
        }
        auto_snake_draw_head(
            game->snake.body[game->snake.head_index]);
        if ((game->state == AUTO_SNAKE_STATE_PLAYING) &&
            game->food_available && game->food_dirty) {
            auto_snake_draw_food(game->food);
        }
    }
    if (game->detached.dirty) {
        for (uint8_t index = 0u;
             index < game->detached.length; index++) {
            auto_snake_draw_cell(
                game->detached.cells[index], false);
        }
        if (game->detached.last_removed_valid) {
            auto_snake_draw_cell(
                game->detached.last_removed_cell, false);
        }
        /*
         * The animated bite cell can refer to the physical segment captured
         * immediately before the circular-buffer head advances. Clear that
         * region explicitly as well; depending on the seek fallback it is
         * not guaranteed to have the same offset in the detached snapshot.
         */
        auto_snake_draw_cell(game->bite_cell, false);
        for (uint8_t offset = 1u;
             offset < game->snake.length; offset++) {
            auto_snake_draw_cell(
                auto_snake_body_at_offset(
                    &game->snake, offset), true);
        }
        auto_snake_draw_head(
            game->snake.body[game->snake.head_index]);
        if (game->food_available) {
            auto_snake_draw_food(game->food);
        }
        if (game->detached.active &&
            game->detached.visible) {
            for (uint8_t index = 0u;
                 index < game->detached.remaining; index++) {
                auto_snake_draw_cell(
                    game->detached.cells[index], true);
            }
        } else if (game->detached.active) {
            for (uint8_t index = 0u;
                 index < game->detached.remaining; index++) {
                auto_snake_draw_blink_cell(
                    game->detached.cells[index]);
            }
        }
    } else if (game->detached.active &&
               (game->redraw_all || game->cut_redraw)) {
        for (uint8_t index = 0u;
             index < game->detached.remaining; index++) {
            if (game->detached.visible) {
                auto_snake_draw_cell(
                    game->detached.cells[index], true);
            } else {
                auto_snake_draw_blink_cell(
                    game->detached.cells[index]);
            }
        }
    }
    if (game->state == AUTO_SNAKE_STATE_BITE_ANIMATION) {
        auto_snake_draw_bite_frame(game);
    }
    game->erased_tail_valid = false;
    game->previous_head_valid = false;
    game->food_dirty = false;
    game->redraw_all = false;
    game->cut_redraw = false;
    game->detached.dirty = false;
    game->detached.last_removed_valid = false;
}
