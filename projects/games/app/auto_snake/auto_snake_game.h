#ifndef __AUTO_SNAKE_GAME_H__
#define __AUTO_SNAKE_GAME_H__

#include <stdbool.h>
#include <stdint.h>

#define AUTO_SNAKE_GRID_WIDTH   (32u)
#define AUTO_SNAKE_GRID_HEIGHT  (16u)
#define AUTO_SNAKE_CELL_PITCH   (4u)
#define AUTO_SNAKE_SEGMENT_SIZE (3u)
#define AUTO_SNAKE_MAX_LENGTH   (60u)
#define AUTO_SNAKE_KEEP_LENGTH  (10u)
#define AUTO_SNAKE_BITE_OFFSET  (10u)
#define AUTO_SNAKE_MAX_DETACHED_LENGTH \
    (AUTO_SNAKE_MAX_LENGTH - AUTO_SNAKE_KEEP_LENGTH)

typedef uint16_t auto_snake_cell_t;

typedef enum {
    AUTO_SNAKE_STATE_PLAYING = 0,
    AUTO_SNAKE_STATE_SEEK_BITE,
    AUTO_SNAKE_STATE_BITE_ANIMATION,
    AUTO_SNAKE_STATE_CUT_BODY,
} auto_snake_state_t;

typedef struct {
    auto_snake_cell_t body[AUTO_SNAKE_MAX_LENGTH];
    uint8_t head_index;
    uint8_t length;
} auto_snake_t;

typedef enum {
    AUTO_SNAKE_DETACHED_NONE = 0,
    AUTO_SNAKE_DETACHED_BLINKING,
    AUTO_SNAKE_DETACHED_DISAPPEARING,
} auto_snake_detached_state_t;

typedef struct {
    auto_snake_cell_t cells[AUTO_SNAKE_MAX_DETACHED_LENGTH];
    uint32_t state_elapsed_ms;
    uint32_t update_accumulator_ms;
    uint8_t length;
    uint8_t remaining;
    auto_snake_detached_state_t state;
    bool visible;
    bool active;
    bool dirty;
} auto_snake_detached_tail_t;

typedef struct {
    auto_snake_t snake;
    auto_snake_detached_tail_t detached;
    uint32_t random_state;
    uint32_t move_accumulator_ms;
    auto_snake_cell_t food;
    auto_snake_cell_t erased_tail;
    auto_snake_cell_t previous_head;
    auto_snake_cell_t bite_cell;
    uint16_t move_interval_ms;
    uint16_t saved_move_interval_ms;
    uint16_t food_count;
    uint16_t bite_count;
    uint16_t bite_seek_steps;
    uint16_t bite_seek_fallbacks;
    uint16_t bite_fallback_steps;
    uint16_t bite_frame_accumulator_ms;
    uint8_t bite_frame;
    uint8_t cut_previous_length;
    uint8_t recovery_steps;
    uint8_t input_previous;
    auto_snake_state_t state;
    bool erased_tail_valid;
    bool previous_head_valid;
    bool food_dirty;
    bool redraw_all;
    bool cut_redraw;
    bool food_available;
    bool bite_pending;
    bool return_to_menu;
} auto_snake_game_t;

_Static_assert(sizeof(auto_snake_t) <= 128u,
               "auto_snake_t uses too much RAM");
_Static_assert(sizeof(auto_snake_game_t) <= 512u,
               "AUTO SNAKE context exceeds Game Manager storage");

void auto_snake_game_init(void *context, uint32_t seed);
void auto_snake_game_deinit(void *context);
void auto_snake_game_update(void *context, uint8_t input_state,
                            uint32_t elapsed_ms);
void auto_snake_game_draw(const void *context);

#endif /* __AUTO_SNAKE_GAME_H__ */
