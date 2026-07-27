#include "game_menu.h"

#include <stdbool.h>
#include <stddef.h>

#include "game_gfx.h"
#include "game_hal.h"
#include "game_registry.h"

#define GAME_MENU_CARD_X          (24u)
#define GAME_MENU_CARD_Y          (1u)
#define GAME_MENU_CARD_WIDTH      (80u)
#define GAME_MENU_CARD_HEIGHT     (62u)
#define GAME_MENU_CENTER_X        (64)
#define GAME_MENU_LEFT_X          (10)
#define GAME_MENU_RIGHT_X         (118)
#define GAME_MENU_ICON_CENTER_Y   (27)
#define GAME_MENU_TRANSITION_STEP (8u)
#define GAME_MENU_TRANSITION_MAX  (64u)

typedef struct {
    uint8_t displayed_index;
    uint8_t previous_index;
    uint8_t transition;
    int8_t direction;
    bool initialized;
} game_menu_animation_t;

static game_menu_animation_t animation;

static void game_menu_draw_rounded_frame(uint8_t x, uint8_t y,
                                         uint8_t width, uint8_t height) {
    /* Two-pixel outline with the broad clipped corners used by the reference. */
    game_display_fill_rect((uint8_t)(x + 6u), y,
                           (uint8_t)(width - 12u), 2u, true);
    game_display_fill_rect((uint8_t)(x + 3u), (uint8_t)(y + 2u),
                           (uint8_t)(width - 6u), 2u, true);
    game_display_fill_rect((uint8_t)(x + 1u), (uint8_t)(y + 4u),
                           2u, 3u, true);
    game_display_fill_rect(x, (uint8_t)(y + 7u), 2u,
                           (uint8_t)(height - 14u), true);
    game_display_fill_rect((uint8_t)(x + width - 2u),
                           (uint8_t)(y + 7u), 2u,
                           (uint8_t)(height - 14u), true);
    game_display_fill_rect((uint8_t)(x + width - 3u),
                           (uint8_t)(y + 4u), 2u, 3u, true);
    game_display_fill_rect((uint8_t)(x + 1u),
                           (uint8_t)(y + height - 7u), 2u, 3u, true);
    game_display_fill_rect((uint8_t)(x + width - 3u),
                           (uint8_t)(y + height - 7u), 2u, 3u, true);
    game_display_fill_rect((uint8_t)(x + 3u),
                           (uint8_t)(y + height - 4u),
                           (uint8_t)(width - 6u), 2u, true);
    game_display_fill_rect((uint8_t)(x + 6u),
                           (uint8_t)(y + height - 2u),
                           (uint8_t)(width - 12u), 2u, true);
}

static void game_menu_draw_edge_arrow(uint8_t x, bool right) {
    const uint8_t center_y = 51u;

    /* Compact chevron: keep it below, not on top of the side icon. */
    for (uint8_t offset = 0u; offset < 4u; offset++) {
        uint8_t draw_x = right ? (uint8_t)(x + 3u - offset) :
                                 (uint8_t)(x + offset);

        game_display_draw_pixel(draw_x,
                                (uint8_t)(center_y - offset), true);
        game_display_draw_pixel(draw_x,
                                (uint8_t)(center_y + offset), true);
    }
}

static uint8_t game_menu_main_width(const game_entry_t *entry) {
    return (uint8_t)(entry->icon_width * entry->icon_scale);
}

static uint8_t game_menu_main_height(const game_entry_t *entry) {
    return (uint8_t)(entry->icon_height * entry->icon_scale);
}

static void game_menu_draw_resized_bitmap(const game_entry_t *entry,
                                          int16_t center_x,
                                          int16_t center_y,
                                          uint8_t target_width,
                                          uint8_t target_height) {
    int16_t origin_x;
    int16_t origin_y;

    if ((entry == NULL) || (entry->icon == NULL) ||
        (target_width == 0u) || (target_height == 0u)) {
        return;
    }
    origin_x = center_x - target_width / 2;
    origin_y = center_y - target_height / 2;

    for (uint8_t y = 0u; y < target_height; y++) {
        uint8_t source_y = (uint8_t)(
            (uint16_t)y * entry->icon_height / target_height);
        for (uint8_t x = 0u; x < target_width; x++) {
            uint8_t source_x = (uint8_t)(
                (uint16_t)x * entry->icon_width / target_width);
            uint8_t data = entry->icon[
                (uint32_t)(source_y / 8u) * entry->icon_width + source_x];
            int16_t draw_x = origin_x + x;
            int16_t draw_y = origin_y + y;

            if ((data & (1u << (source_y & 7u))) != 0u &&
                (draw_x >= 0) &&
                (draw_x < (int16_t)GAME_DISPLAY_WIDTH) &&
                (draw_y >= 7) && (draw_y < 47)) {
                game_display_draw_pixel((uint8_t)draw_x,
                                        (uint8_t)draw_y, true);
            }
        }
    }
}

static void game_menu_draw_icon(const game_entry_t *entry,
                                int16_t center_x, uint8_t scale_64) {
    uint8_t main_width;
    uint8_t main_height;
    uint8_t target_width;
    uint8_t target_height;

    if (entry == NULL) {
        return;
    }
    main_width = game_menu_main_width(entry);
    main_height = game_menu_main_height(entry);
    /* scale_64 runs from 32 (side icon) to 64 (main icon). */
    target_width = (uint8_t)((uint16_t)main_width * scale_64 / 64u);
    target_height = (uint8_t)((uint16_t)main_height * scale_64 / 64u);
    game_menu_draw_resized_bitmap(entry, center_x, GAME_MENU_ICON_CENTER_Y,
                                  target_width, target_height);
}

static void game_menu_draw_name(const char *name) {
    char text[13];
    uint8_t length = 0u;
    uint8_t x;

    while ((name[length] != '\0') && (length < 12u)) {
        text[length] = name[length];
        length++;
    }
    text[length] = '\0';
    x = (uint8_t)((GAME_DISPLAY_WIDTH - (uint16_t)length * 6u) / 2u);
    game_gfx_draw_text(x, 50u, text, 1u, true, false);
}

static void game_menu_draw_position(uint8_t selected_index) {
    uint8_t dot_count = (uint8_t)(selected_index + 1u);

    for (uint8_t dot = 0u; dot < dot_count; dot++) {
        game_display_fill_rect((uint8_t)(30u + dot * 4u), 6u,
                               2u, 2u, true);
    }
}

static void game_menu_begin_transition(uint8_t selected_index, size_t count) {
    if (!animation.initialized) {
        animation.displayed_index = selected_index;
        animation.previous_index = selected_index;
        animation.transition = GAME_MENU_TRANSITION_MAX;
        animation.direction = 1;
        animation.initialized = true;
        return;
    }
    if (selected_index == animation.displayed_index) {
        return;
    }

    animation.previous_index = animation.displayed_index;
    animation.direction =
        selected_index == (uint8_t)((animation.displayed_index + 1u) % count) ?
        1 : -1;
    animation.displayed_index = selected_index;
    animation.transition = 0u;
}

static uint8_t game_menu_previous_index(uint8_t index, size_t count) {
    return (index == 0u) ? (uint8_t)(count - 1u) : (uint8_t)(index - 1u);
}

static uint8_t game_menu_next_index(uint8_t index, size_t count) {
    return (uint8_t)((index + 1u) % count);
}

static int16_t game_menu_lerp(int16_t from, int16_t to, uint8_t progress) {
    return (int16_t)(from + (to - from) * progress /
                            GAME_MENU_TRANSITION_MAX);
}

static void game_menu_draw_resting(uint8_t selected_index, size_t count) {
    game_menu_draw_icon(game_registry_get(selected_index),
                        GAME_MENU_CENTER_X, 64u);
    game_menu_draw_icon(
        game_registry_get(game_menu_previous_index(selected_index, count)),
        GAME_MENU_LEFT_X, 32u);
    game_menu_draw_icon(
        game_registry_get(game_menu_next_index(selected_index, count)),
        GAME_MENU_RIGHT_X, 32u);
}

static void game_menu_draw_transition(size_t count) {
    uint8_t progress = animation.transition;
    int16_t old_target = (animation.direction > 0) ?
                         GAME_MENU_LEFT_X : GAME_MENU_RIGHT_X;
    int16_t new_source = (animation.direction > 0) ?
                         GAME_MENU_RIGHT_X : GAME_MENU_LEFT_X;
    int16_t old_x = game_menu_lerp(GAME_MENU_CENTER_X, old_target, progress);
    int16_t new_x = game_menu_lerp(new_source, GAME_MENU_CENTER_X, progress);
    uint8_t old_scale = (uint8_t)(64u - progress / 2u);
    uint8_t new_scale = (uint8_t)(32u + progress / 2u);
    uint8_t far_index;
    int16_t far_x;

    game_menu_draw_icon(game_registry_get(animation.previous_index),
                        old_x, old_scale);
    game_menu_draw_icon(game_registry_get(animation.displayed_index),
                        new_x, new_scale);

    /* Keep the opposite neighbour visible throughout the transition. */
    if (animation.direction > 0) {
        far_index = game_menu_next_index(animation.displayed_index, count);
        far_x = GAME_MENU_RIGHT_X;
    } else {
        far_index = game_menu_previous_index(animation.displayed_index, count);
        far_x = GAME_MENU_LEFT_X;
    }
    game_menu_draw_icon(game_registry_get(far_index), far_x, 32u);
}

void game_menu_draw(uint8_t selected_index) {
    size_t count = game_registry_count();
    const game_entry_t *selected;

    game_display_clear();
    if (count == 0u) {
        return;
    }
    game_menu_begin_transition(selected_index, count);
    selected = game_registry_get(animation.displayed_index);

    game_menu_draw_edge_arrow(0u, false);
    game_menu_draw_edge_arrow(124u, true);

    if (animation.transition < GAME_MENU_TRANSITION_MAX) {
        game_menu_draw_transition(count);
        if (animation.transition >= GAME_MENU_TRANSITION_MAX / 2u) {
            game_menu_draw_name(selected->name);
        } else {
            game_menu_draw_name(
                game_registry_get(animation.previous_index)->name);
        }
        animation.transition =
            (uint8_t)(animation.transition + GAME_MENU_TRANSITION_STEP);
    } else {
        game_menu_draw_resting(animation.displayed_index, count);
        game_menu_draw_name(selected->name);
    }

    /* Keep the card above moving artwork as it crosses the side boundary. */
    game_menu_draw_rounded_frame(GAME_MENU_CARD_X, GAME_MENU_CARD_Y,
                                 GAME_MENU_CARD_WIDTH, GAME_MENU_CARD_HEIGHT);
    game_menu_draw_position(animation.displayed_index);
}
