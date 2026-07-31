#include "game_manager.h"

#include <string.h>

#include "app.h"
#include "game_menu.h"

static bool game_manager_select_auto_snake(game_manager_t *manager) {
    size_t count = game_registry_count();

    for (size_t index = 0u; index < count; index++) {
        const game_entry_t *entry = game_registry_get(index);

        if ((entry != NULL) &&
            (strcmp(entry->name, "AUTO SNAKE") == 0)) {
            manager->selected_index = (uint8_t)index;
            return true;
        }
    }
    return false;
}

static void game_manager_start_selected(game_manager_t *manager) {
    const game_entry_t *entry = game_registry_get(manager->selected_index);

    if ((entry == NULL) || (entry->context_size > sizeof(manager->context.bytes))) {
        return;
    }
    memset(manager->context.bytes, 0, sizeof(manager->context.bytes));
    manager->active_game = entry;
    manager->navigation_hold_updates = 0u;
    manager->menu_idle_ms = 0u;
    manager->input_locked = true;
    manager->mode = GAME_MANAGER_RUNNING;
    entry->init(manager->context.bytes, manager->uptime_ms);
}

static void game_manager_move_selection(game_manager_t *manager,
                                        int8_t direction, size_t count) {
    if ((manager == NULL) || (count == 0u)) {
        return;
    }
    if (direction < 0) {
        manager->selected_index = (manager->selected_index == 0u) ?
            (uint8_t)(count - 1u) : (uint8_t)(manager->selected_index - 1u);
    } else {
        manager->selected_index =
            (uint8_t)((manager->selected_index + 1u) % count);
    }
}

void game_manager_init(game_manager_t *manager) {
    if (manager == NULL) {
        return;
    }

    memset(manager, 0, sizeof(*manager));
    manager->mode = GAME_MANAGER_MENU;
}

void game_manager_update(game_manager_t *manager, uint8_t input_state,
                         uint32_t elapsed_ms) {
    uint8_t pressed;
    uint8_t navigation;

    if (manager == NULL) {
        return;
    }
    manager->uptime_ms += elapsed_ms;
    if (manager->input_locked) {
        manager->input_previous = input_state;
        if (input_state == 0u) {
            manager->input_locked = false;
            manager->input_previous = 0u;
        }
        return;
    }

    pressed = (uint8_t)(input_state & ~manager->input_previous);
    manager->input_previous = input_state;

    if (manager->mode == GAME_MANAGER_MENU) {
        size_t count = game_registry_count();

        if (count == 0u) {
            return;
        }
        if (input_state != 0u) {
            manager->menu_idle_ms = 0u;
        } else if (manager->menu_idle_ms < GAME_MANAGER_AUTO_SNAKE_IDLE_MS) {
            manager->menu_idle_ms += elapsed_ms;
            if (manager->menu_idle_ms >= GAME_MANAGER_AUTO_SNAKE_IDLE_MS) {
                if (game_manager_select_auto_snake(manager)) {
                    game_manager_start_selected(manager);
                    return;
                }
                manager->menu_idle_ms = 0u;
            }
        }
        navigation = input_state & (GAME_INPUT_UP | GAME_INPUT_DOWN);
        if ((navigation == GAME_INPUT_UP) ||
            (navigation == GAME_INPUT_DOWN)) {
            bool repeat = false;

            if (manager->navigation_hold_updates < UINT8_MAX) {
                manager->navigation_hold_updates++;
            }
            if (manager->navigation_hold_updates >=
                GAME_MANAGER_REPEAT_DELAY_UPDATES) {
                repeat = ((manager->navigation_hold_updates -
                           GAME_MANAGER_REPEAT_DELAY_UPDATES) %
                          GAME_MANAGER_REPEAT_PERIOD_UPDATES) == 0u;
            }
            if (((pressed & navigation) != 0u) || repeat) {
                game_manager_move_selection(
                    manager, navigation == GAME_INPUT_UP ? -1 : 1, count);
            }
        } else {
            manager->navigation_hold_updates = 0u;
        }
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game_manager_start_selected(manager);
        }
        return;
    }

    if (manager->active_game != NULL) {
        manager->active_game->update(manager->context.bytes, input_state,
                                     elapsed_ms);
        if ((manager->active_game->wants_menu != NULL) &&
            manager->active_game->wants_menu(manager->context.bytes)) {
            if (manager->active_game->deinit != NULL) {
                manager->active_game->deinit(manager->context.bytes);
            }
            manager->active_game = NULL;
            manager->mode = GAME_MANAGER_MENU;
            manager->menu_idle_ms = 0u;
            manager->navigation_hold_updates = 0u;
            manager->input_locked = true;
            manager->input_previous = input_state;
        }
    }
}

void game_manager_draw(const game_manager_t *manager) {
    if (manager == NULL) {
        return;
    }
    if (manager->mode == GAME_MANAGER_MENU) {
        game_menu_draw(manager->selected_index);
    } else if (manager->active_game != NULL) {
        manager->active_game->draw(manager->context.bytes);
    }
}

uint8_t game_manager_take_events(game_manager_t *manager) {
    if ((manager == NULL) || (manager->mode != GAME_MANAGER_RUNNING) ||
        (manager->active_game == NULL) ||
        (manager->active_game->take_events == NULL)) {
        return 0u;
    }
    return manager->active_game->take_events(manager->context.bytes);
}
