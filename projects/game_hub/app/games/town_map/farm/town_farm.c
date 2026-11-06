/**
 ******************************************************************************
 * @file    town_farm.c
 * @brief   Town farm module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "town_farm.h"

#include "graphics.h"

#include <stddef.h>

#include "game_input.h"
#include "bitmap_viewer_bitmaps.h"
#include "farm_bitmaps.h"
#include "town_areas.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define TOWN_FARM_BASE_PLACEMENTS (28u)
#define TOWN_FARM_TOOL_COUNT      (4u)
#define TOWN_FARM_CURSOR_COUNT    (TOWN_FARM_PLOT_COUNT + 1u)
#define TOWN_FARM_FOCUS_TOOLBAR   (0u)
#define TOWN_FARM_FOCUS_PLOTS     (1u)
#define TOWN_FARM_TOOL_WATER      (0u)
#define TOWN_FARM_TOOL_HOE        (1u)
#define TOWN_FARM_TOOL_SEED       (2u)
#define TOWN_FARM_TOOL_EXIT       (3u)
#define TOWN_FARM_GROWTH_STAGE_MS (15000u)
#define TOWN_FARM_TOOL_BLINK_MS   (400u)
#define TOWN_FARM_CROP_TYPE_MASK  (0x03u)
#define TOWN_FARM_WATERED_MASK    (0x80u)
#define TOWN_FARM_MATURE_STAGE    (5u)
#define TOWN_FARM_ROOT_OFFSET_Y   (11u)

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef struct {
    uint8_t x;
    uint8_t y;
} town_farm_plot_position_t;

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/

static const town_farm_plot_position_t town_farm_plot_positions[TOWN_FARM_PLOT_COUNT] = {
    {14u, 14u}, {33u, 14u}, {52u, 14u}, {71u, 14u}, {90u, 14u},
    {24u, 33u}, {43u, 33u}, {62u, 33u}, {81u, 33u}, {100u, 33u},
};

static const town_farm_plot_position_t town_farm_tool_positions[TOWN_FARM_TOOL_COUNT] = {
    {36u, 51u},
    {52u, 51u},
    {68u, 51u},
    {81u, 51u},
};

static const town_farm_plot_position_t town_farm_cancel_position = {5u, 33u};

static void town_farm_draw_bitmap(uint8_t x, uint8_t y, const bitmap_viewer_entry_t *entry);

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static uint8_t town_farm_cursor_to_plot(uint8_t cursor) {
    return (cursor < TOWN_FARM_CANCEL_INDEX) ? cursor : (uint8_t)(cursor - 1u);
}

static const town_farm_plot_position_t *town_farm_cursor_position(uint8_t cursor) {
    if (cursor == TOWN_FARM_CANCEL_INDEX) {
        return &town_farm_cancel_position;
    }
    return &town_farm_plot_positions[town_farm_cursor_to_plot(cursor)];
}

static void town_farm_draw_cancel_x(void) {
    for (uint8_t y = 33u; y < 48u; y++) {
        for (uint8_t x = 5u; x < 20u; x++) {
            gfx_draw_pixel(x, y, false);
        }
    }
    town_farm_draw_bitmap(5u, 33u, &farm_plot_bitmap);
    for (uint8_t offset = 0u; offset < 9u; offset++) {
        gfx_draw_pixel((uint8_t)(8u + offset), (uint8_t)(36u + offset), true);
        gfx_draw_pixel((uint8_t)(16u - offset), (uint8_t)(36u + offset), true);
    }
}

static void town_farm_draw_bitmap(uint8_t x, uint8_t y, const bitmap_viewer_entry_t *entry) {
    for (uint8_t row = 0u; row < entry->height; row++) {
        for (uint8_t column = 0u; column < entry->width; column++) {
            uint8_t value = entry->data[(uint16_t)row * entry->stride + column / 8u];

            if ((value & (uint8_t)(0x80u >> (column & 7u))) != 0u) {
                gfx_draw_pixel((uint8_t)(x + column), (uint8_t)(y + row), true);
            }
        }
    }
}

static void town_farm_clear_bitmap_area(uint8_t x, uint8_t y, const bitmap_viewer_entry_t *entry) {
    for (uint8_t row = 0u; row < entry->height; row++) {
        for (uint8_t column = 0u; column < entry->width; column++) {
            gfx_draw_pixel((uint8_t)(x + column), (uint8_t)(y + row), false);
        }
    }
}

static void town_farm_draw_bitmap_with_black_outline(uint8_t x, uint8_t y,
                                                     const bitmap_viewer_entry_t *entry) {
    for (uint8_t row = 0u; row < entry->height; row++) {
        for (uint8_t column = 0u; column < entry->width; column++) {
            uint8_t value = entry->data[(uint16_t)row * entry->stride + column / 8u];

            if ((value & (uint8_t)(0x80u >> (column & 7u))) == 0u) {
                continue;
            }
            for (int8_t offset_y = -1; offset_y <= 1; offset_y++) {
                for (int8_t offset_x = -1; offset_x <= 1; offset_x++) {
                    int16_t pixel_x = (int16_t)x + column + offset_x;
                    int16_t pixel_y = (int16_t)y + row + offset_y;

                    if ((pixel_x >= 0) && (pixel_x < (int16_t)GFX_WIDTH) && (pixel_y >= 0) &&
                        (pixel_y < (int16_t)GFX_HEIGHT)) {
                        gfx_draw_pixel((uint8_t)pixel_x, (uint8_t)pixel_y, false);
                    }
                }
            }
        }
    }
    town_farm_draw_bitmap(x, y, entry);
}

static void town_farm_apply_action(town_map_game_t *game) {
    uint8_t plot = town_farm_cursor_to_plot(game->farm_selected_plot);
    uint8_t *stage = &game->farm_plots[plot];
    uint8_t *crop = &game->farm_crop_types[plot];

    if (game->farm_selected_tool == TOWN_FARM_TOOL_HOE) {
        if ((*stage == TOWN_FARM_MATURE_STAGE) && (game->coin_count < UINT8_MAX)) {
            game->coin_count =
                (game->coin_count <= UINT8_MAX - 3u) ? (uint8_t)(game->coin_count + 3u) : UINT8_MAX;
        }
        *stage = 0u;
        *crop = 0u;
        game->farm_growth_ms[plot] = 0u;
    } else if (game->farm_selected_tool == TOWN_FARM_TOOL_SEED) {
        if ((*stage == 0u) && (game->coin_count > 0u)) {
            /* TODO: Add a seed-selection menu instead of random seed type. */
            uint32_t random = game->farm_random_state;

            random ^= random << 13u;
            random ^= random >> 17u;
            random ^= random << 5u;
            game->farm_random_state = random;
            *crop = (uint8_t)(1u + random % 3u);
            *stage = 1u;
            game->coin_count--;
            game->farm_growth_ms[plot] = 0u;
        }
    } else if ((*stage > 0u) && (*stage < TOWN_FARM_MATURE_STAGE)) {
        if (*stage == 1u) {
            /* First watering creates soft soil, then growth starts itself. */
            *stage = 2u;
            *crop = (uint8_t)((*crop & TOWN_FARM_CROP_TYPE_MASK) | TOWN_FARM_WATERED_MASK);
            game->farm_growth_ms[plot] = 0u;
        } else if ((*crop & TOWN_FARM_WATERED_MASK) == 0u) {
            *crop |= TOWN_FARM_WATERED_MASK;
            game->farm_growth_ms[plot] = 0u;
        }
    }
}

static void town_farm_update_growth(town_map_game_t *game, uint32_t elapsed_ms) {
    for (uint8_t plot = 0u; plot < TOWN_FARM_PLOT_COUNT; plot++) {
        if ((game->farm_plots[plot] == 0u) || (game->farm_plots[plot] >= TOWN_FARM_MATURE_STAGE) ||
            ((game->farm_crop_types[plot] & TOWN_FARM_WATERED_MASK) == 0u)) {
            continue;
        }
        if (elapsed_ms >= TOWN_FARM_GROWTH_STAGE_MS - game->farm_growth_ms[plot]) {
            game->farm_plots[plot]++;
            game->farm_crop_types[plot] &= TOWN_FARM_CROP_TYPE_MASK;
            game->farm_growth_ms[plot] = 0u;
        } else {
            game->farm_growth_ms[plot] = (uint16_t)(game->farm_growth_ms[plot] + elapsed_ms);
        }
    }
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void town_farm_update(town_map_game_t *game, uint8_t input_state, uint8_t pressed,
                      uint32_t elapsed_ms) {
    uint8_t *cursor;
    uint8_t cursor_count;

    (void)input_state;
    game->farm_clock_accumulator_ms += elapsed_ms;
    while (game->farm_clock_accumulator_ms >= 5000u) {
        game->farm_clock_accumulator_ms -= 5000u;
        game->farm_minutes = (uint16_t)((game->farm_minutes + 1u) % 1440u);
    }
    town_farm_update_growth(game, elapsed_ms);
    cursor = (game->farm_focus == TOWN_FARM_FOCUS_TOOLBAR) ? &game->farm_selected_tool
                                                           : &game->farm_selected_plot;
    cursor_count = (game->farm_focus == TOWN_FARM_FOCUS_TOOLBAR) ? TOWN_FARM_TOOL_COUNT
                                                                 : TOWN_FARM_CURSOR_COUNT;

    if ((pressed & GAME_INPUT_UP) != 0u) {
        *cursor = (*cursor == 0u) ? (uint8_t)(cursor_count - 1u) : (uint8_t)(*cursor - 1u);
    }
    if ((pressed & GAME_INPUT_DOWN) != 0u) {
        *cursor = (uint8_t)((*cursor + 1u) % cursor_count);
    }
    if ((pressed & GAME_INPUT_MODE) != 0u) {
        if (game->farm_focus == TOWN_FARM_FOCUS_TOOLBAR) {
            if (game->farm_selected_tool == TOWN_FARM_TOOL_EXIT) {
                town_area_leave_farm(game);
            } else {
                game->farm_focus = TOWN_FARM_FOCUS_PLOTS;
                game->farm_selected_plot = 0u;
            }
        } else if (game->farm_selected_plot == TOWN_FARM_CANCEL_INDEX) {
            game->farm_focus = TOWN_FARM_FOCUS_TOOLBAR;
        } else {
            town_farm_apply_action(game);
        }
    }
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void town_farm_draw_digit(uint8_t x, uint8_t y, uint8_t digit) {
    static const uint8_t digits[10u][5u] = {
        {7u, 5u, 5u, 5u, 7u}, {2u, 6u, 2u, 2u, 7u}, {7u, 1u, 7u, 4u, 7u}, {7u, 1u, 7u, 1u, 7u},
        {5u, 5u, 7u, 1u, 1u}, {7u, 4u, 7u, 1u, 7u}, {7u, 4u, 7u, 5u, 7u}, {7u, 1u, 1u, 1u, 1u},
        {7u, 5u, 7u, 5u, 7u}, {7u, 5u, 7u, 1u, 7u},
    };

    for (uint8_t row = 0u; row < 5u; row++) {
        for (uint8_t column = 0u; column < 3u; column++) {
            if ((digits[digit][row] & (uint8_t)(4u >> column)) != 0u) {
                gfx_draw_pixel((uint8_t)(x + column), (uint8_t)(y + row), true);
            }
        }
    }
}

static void town_farm_draw_clock(const town_map_game_t *game) {
    uint8_t hour = (uint8_t)(game->farm_minutes / 60u);
    uint8_t minute = (uint8_t)(game->farm_minutes % 60u);
    bool daytime = (hour >= 6u) && (hour < 18u);

    town_farm_draw_bitmap(daytime ? 44u : 46u, 2u, daytime ? &farm_sun_bitmap : &farm_moon_bitmap);
    town_farm_draw_digit(55u, 3u, (uint8_t)(hour / 10u));
    town_farm_draw_digit(59u, 3u, (uint8_t)(hour % 10u));
    gfx_draw_pixel(63u, 4u, true);
    gfx_draw_pixel(63u, 6u, true);
    town_farm_draw_digit(65u, 3u, (uint8_t)(minute / 10u));
    town_farm_draw_digit(69u, 3u, (uint8_t)(minute % 10u));
}

static const bitmap_viewer_entry_t *town_farm_crop_for_stage(uint8_t stage, uint8_t crop_type) {
    if (stage == 1u) {
        return &farm_sprout_bitmap;
    }
    if (stage == 2u) {
        return &farm_soft_soil_bitmap;
    }
    if (stage == 3u) {
        return &farm_two_leaves_bitmap;
    }
    if (stage == 4u) {
        return &farm_flower_bitmap;
    }
    if (stage == TOWN_FARM_MATURE_STAGE) {
        switch (crop_type & TOWN_FARM_CROP_TYPE_MASK) {
            case 1u:
                return &farm_wheat_bitmap;
            case 2u:
                return &farm_pumpkin_bitmap;
            default:
                return &farm_tomato_bitmap;
        }
    }
    return NULL;
}

static void town_farm_draw_water_needed(uint8_t x, uint8_t y) {
    const bitmap_viewer_entry_t *watering_can = &farm_watering_can_bitmap;

    town_farm_draw_bitmap_with_black_outline(x, y, watering_can);
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void town_farm_draw(const town_map_game_t *game) {
    const town_farm_plot_position_t *selected;
    bool hide_active_tool =
        (game->farm_focus == TOWN_FARM_FOCUS_PLOTS) &&
        (((game->farm_clock_accumulator_ms / TOWN_FARM_TOOL_BLINK_MS) & 1u) != 0u);

    gfx_clear();
    for (size_t index = 0u;
         (index < TOWN_FARM_BASE_PLACEMENTS) && (index < bitmap_viewer_farm_layout_count);
         index++) {
        const bitmap_viewer_placement_t *placement = &bitmap_viewer_farm_layout[index];

        if (((index >= 9u) && (index <= 24u)) || (index >= 26u)) {
            continue;
        }
        town_farm_draw_bitmap(placement->x, placement->y, &placement->bitmap);
    }
    for (uint8_t index = 0u; index < TOWN_FARM_PLOT_COUNT; index++) {
        town_farm_draw_bitmap(town_farm_plot_positions[index].x, town_farm_plot_positions[index].y,
                              &farm_plot_bitmap);
    }
    if (!hide_active_tool || (game->farm_selected_tool != TOWN_FARM_TOOL_WATER)) {
        town_farm_draw_bitmap(38u, 53u, &farm_watering_can_bitmap);
    }
    if (!hide_active_tool || (game->farm_selected_tool != TOWN_FARM_TOOL_HOE)) {
        town_farm_draw_bitmap(54u, 52u, &farm_hoe_bitmap);
    }
    if (!hide_active_tool || (game->farm_selected_tool != TOWN_FARM_TOOL_SEED)) {
        town_farm_draw_bitmap(70u, 52u, &farm_seed_bag_bitmap);
    }
    town_farm_draw_bitmap(83u, 52u, &farm_exit_bitmap);

    for (uint8_t index = 0u; index < TOWN_FARM_PLOT_COUNT; index++) {
        const bitmap_viewer_entry_t *crop =
            town_farm_crop_for_stage(game->farm_plots[index], game->farm_crop_types[index]);
        const town_farm_plot_position_t *plot = &town_farm_plot_positions[index];

        if (crop != NULL) {
            uint8_t crop_x = (uint8_t)(plot->x + (15u - crop->width) / 2u);
            uint8_t crop_y = (uint8_t)(plot->y + TOWN_FARM_ROOT_OFFSET_Y + 1u - crop->height);

            if (game->farm_plots[index] == 1u) {
                crop_y--;
            }

            town_farm_clear_bitmap_area(crop_x, crop_y, crop);
            town_farm_draw_bitmap(crop_x, crop_y, crop);
        }
        if ((game->farm_plots[index] > 0u) && (game->farm_plots[index] < TOWN_FARM_MATURE_STAGE) &&
            ((game->farm_crop_types[index] & TOWN_FARM_WATERED_MASK) == 0u)) {
            town_farm_draw_water_needed((uint8_t)(plot->x + 6u), (uint8_t)(plot->y - 5u));
        }
    }
    town_farm_draw_clock(game);

    if (game->farm_focus == TOWN_FARM_FOCUS_TOOLBAR) {
        selected = &town_farm_tool_positions[game->farm_selected_tool];
        gfx_draw_rect(selected->x, selected->y, 15u, 13u, true);
    } else {
        town_farm_draw_cancel_x();
        selected = town_farm_cursor_position(game->farm_selected_plot);
        gfx_draw_rect((uint8_t)(selected->x - 1u), (uint8_t)(selected->y - 1u), 17u, 17u, true);
    }
    town_farm_draw_coin_counter(game, 2u, 54u);
}

void town_farm_draw_coin_counter(const town_map_game_t *game, uint8_t x, uint8_t y) {
    for (uint8_t row = 0u; row < 9u; row++) {
        for (uint8_t column = 0u; column < 27u; column++) {
            uint16_t pixel_x = (uint16_t)x + column;
            uint16_t pixel_y = (uint16_t)y + row;

            if ((pixel_x < GFX_WIDTH) && (pixel_y < GFX_HEIGHT)) {
                gfx_draw_pixel((uint8_t)pixel_x, (uint8_t)pixel_y, false);
            }
        }
    }
    town_farm_draw_bitmap_with_black_outline(x, y, &farm_coin_bitmap);
    gfx_draw_number((uint8_t)(x + 9u), y, game->coin_count, 1u, true, false);
}
