#ifndef __GAME_REGISTRY_H__
#define __GAME_REGISTRY_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef void (*game_init_fn_t)(void *context, uint32_t seed);
typedef void (*game_deinit_fn_t)(void *context);
typedef void (*game_update_fn_t)(void *context, uint8_t input_state,
                                 uint32_t elapsed_ms);
typedef void (*game_draw_fn_t)(const void *context);
typedef uint8_t (*game_take_events_fn_t)(void *context);
typedef bool (*game_wants_menu_fn_t)(const void *context);

typedef struct {
    const char *name;
    const uint8_t *icon;
    uint8_t icon_width;
    uint8_t icon_height;
    uint8_t icon_scale;
    size_t context_size;
    game_init_fn_t init;
    game_deinit_fn_t deinit;
    game_update_fn_t update;
    game_draw_fn_t draw;
    game_take_events_fn_t take_events;
    game_wants_menu_fn_t wants_menu;
} game_entry_t;

size_t game_registry_count(void);
const game_entry_t *game_registry_get(size_t index);

#endif /* __GAME_REGISTRY_H__ */
