/**
 ******************************************************************************
 * @file    town_areas.c
 * @brief   Town areas module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "town_areas.h"

#include "graphics.h"

#include <stddef.h>

#include "town_area_bitmaps.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define TOWN_MAIN_VERTICAL_X   ((TOWN_WORLD_WIDTH - TOWN_ROAD_WIDTH) / 2)
#define TOWN_MAIN_HORIZONTAL_Y ((TOWN_WORLD_HEIGHT - TOWN_ROAD_WIDTH) / 2)
#define TOWN_BRANCH_Y          (184)
#define TOWN_FARM_BRANCH_X     (96)
#define TOWN_FARM_BRANCH_END   (TOWN_MAIN_VERTICAL_X)
#define TOWN_POND_BRANCH_X     (TOWN_MAIN_VERTICAL_X + TOWN_ROAD_WIDTH)
#define TOWN_POND_BRANCH_END   (224)
#define TOWN_AREA_PATH_X       ((TOWN_WORLD_WIDTH - TOWN_BRANCH_WIDTH) / 2)
#define TOWN_AREA_PATH_Y       (112)

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static bool town_area_in_world(int16_t x, int16_t y) {
    return (x >= 0) && (x < TOWN_WORLD_WIDTH) && (y >= 0) && (y < TOWN_WORLD_HEIGHT);
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
bool town_area_town_point_on_road(int16_t x, int16_t y) {
    bool vertical = (x >= TOWN_MAIN_VERTICAL_X) && (x < TOWN_MAIN_VERTICAL_X + TOWN_ROAD_WIDTH);
    bool horizontal =
        (y >= TOWN_MAIN_HORIZONTAL_Y) && (y < TOWN_MAIN_HORIZONTAL_Y + TOWN_ROAD_WIDTH);
    bool branch_y = (y >= TOWN_BRANCH_Y) && (y < TOWN_BRANCH_Y + TOWN_BRANCH_WIDTH);
    bool farm_branch = branch_y && (x >= TOWN_FARM_BRANCH_X) && (x < TOWN_FARM_BRANCH_END);
    bool pond_branch = branch_y && (x >= TOWN_POND_BRANCH_X) && (x < TOWN_POND_BRANCH_END);

    return town_area_in_world(x, y) && (vertical || horizontal || farm_branch || pond_branch);
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static bool town_area_farm_walkable(int16_t x, int16_t y) {
    bool vertical = (x >= TOWN_AREA_PATH_X) && (x < TOWN_AREA_PATH_X + TOWN_BRANCH_WIDTH);
    bool horizontal = (y >= TOWN_AREA_PATH_Y) && (y < TOWN_AREA_PATH_Y + TOWN_BRANCH_WIDTH);

    return town_area_in_world(x, y) && (vertical || horizontal);
}

static bool town_area_pond_walkable(int16_t x, int16_t y) {
    bool entrance =
        (x >= TOWN_AREA_PATH_X) && (x < TOWN_AREA_PATH_X + TOWN_BRANCH_WIDTH) && (y >= 146);
    bool pond_top = (y >= 38) && (y < 62) && (x >= 66) && (x < 254);
    bool pond_bottom = (y >= 134) && (y < 158) && (x >= 66) && (x < 254);
    bool pond_left = (x >= 66) && (x < 90) && (y >= 38) && (y < 158);
    bool pond_right = (x >= 230) && (x < 254) && (y >= 38) && (y < 158);

    return town_area_in_world(x, y) &&
           (entrance || pond_top || pond_bottom || pond_left || pond_right);
}

static bool town_area_point_walkable(town_scene_t scene, int16_t x, int16_t y) {
    if (scene == TOWN_SCENE_FARM) {
        return town_area_farm_walkable(x, y);
    }
    if (scene == TOWN_SCENE_FISH_POND) {
        return town_area_pond_walkable(x, y);
    }
    return town_area_town_point_on_road(x, y);
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
bool town_area_player_fits(town_scene_t scene, int16_t x, int16_t y) {
    int16_t collision_top = (scene == TOWN_SCENE_TOWN) ? (int16_t)(y + TOWN_PLAYER_HEIGHT / 2) : y;

    return town_area_point_walkable(scene, x, collision_top) &&
           town_area_point_walkable(scene, (int16_t)(x + TOWN_PLAYER_WIDTH - 1), collision_top) &&
           town_area_point_walkable(scene, x, (int16_t)(y + TOWN_PLAYER_HEIGHT - 1)) &&
           town_area_point_walkable(scene, (int16_t)(x + TOWN_PLAYER_WIDTH - 1),
                                    (int16_t)(y + TOWN_PLAYER_HEIGHT - 1));
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void town_area_reset_motion(town_map_game_t *game) {
    game->move_accumulator_ms = 0u;
    game->animation_accumulator_ms = 0u;
    game->player.animation_frame = 0u;
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void town_area_enter_farm(town_map_game_t *game) {
    game->scene = TOWN_SCENE_FARM;
    game->farm_focus = 0u;
    game->farm_selected_tool = 0u;
    game->farm_selected_plot = 0u;
    if (game->farm_minutes == 0u) {
        game->farm_minutes = 360u;
    }
    game->player.x = (int16_t)(TOWN_WORLD_WIDTH / 2 - TOWN_PLAYER_WIDTH / 2);
    game->player.y = (int16_t)(TOWN_WORLD_HEIGHT - TOWN_PLAYER_HEIGHT - 1);
    game->player.direction = TOWN_DIR_UP;
    game->area_intro_ms = 1000u;
    town_area_reset_motion(game);
}

void town_area_enter_fish_pond(town_map_game_t *game) {
    game->scene = TOWN_SCENE_FISH_POND;
    game->fish_selected = 0u;
    game->player.x = (int16_t)(TOWN_WORLD_WIDTH / 2 - TOWN_PLAYER_WIDTH / 2);
    game->player.y = (int16_t)(TOWN_WORLD_HEIGHT - TOWN_PLAYER_HEIGHT - 1);
    game->player.direction = TOWN_DIR_UP;
    game->area_intro_ms = 1000u;
    town_area_reset_motion(game);
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void town_area_return_to_town(town_map_game_t *game, bool from_farm) {
    game->scene = TOWN_SCENE_TOWN;
    game->player.x = from_farm ? 98 : 206;
    game->player.y = (int16_t)(TOWN_BRANCH_Y + (TOWN_BRANCH_WIDTH - TOWN_PLAYER_HEIGHT) / 2);
    game->player.direction = from_farm ? TOWN_DIR_RIGHT : TOWN_DIR_LEFT;
    town_area_reset_motion(game);
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void town_area_leave_farm(town_map_game_t *game) {
    town_area_return_to_town(game, true);
    game->player.state = TOWN_PLAYER_STANDING;
}

void town_area_leave_fish_pond(town_map_game_t *game) {
    town_area_return_to_town(game, false);
    game->player.state = TOWN_PLAYER_STANDING;
}

bool town_area_check_transition(town_map_game_t *game) {
    if (game->scene == TOWN_SCENE_TOWN) {
        if ((game->player.y >= TOWN_BRANCH_Y) &&
            (game->player.y + TOWN_PLAYER_HEIGHT <= TOWN_BRANCH_Y + TOWN_BRANCH_WIDTH)) {
            if (game->player.x <= TOWN_FARM_BRANCH_X) {
                town_area_enter_farm(game);
                return true;
            }
            if (game->player.x >= TOWN_POND_BRANCH_END - TOWN_PLAYER_WIDTH) {
                town_area_enter_fish_pond(game);
                return true;
            }
        }
        return false;
    }

    if ((game->player.direction == TOWN_DIR_DOWN) &&
        (game->player.y >= TOWN_WORLD_HEIGHT - TOWN_PLAYER_HEIGHT - 1)) {
        town_area_return_to_town(game, game->scene == TOWN_SCENE_FARM);
        return true;
    }
    return false;
}

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void town_area_pixel(int16_t world_x, int16_t world_y, int16_t camera_x, int16_t camera_y,
                            bool on) {
    int16_t screen_x = (int16_t)(world_x - camera_x);
    int16_t screen_y = (int16_t)(world_y - camera_y);

    if ((screen_x >= 0) && (screen_x < (int16_t)GFX_WIDTH) && (screen_y >= 0) &&
        (screen_y < (int16_t)GFX_HEIGHT)) {
        gfx_draw_pixel((uint8_t)screen_x, (uint8_t)screen_y, on);
    }
}

static bool town_area_rect_edge(int16_t x, int16_t y, int16_t left, int16_t top, int16_t width,
                                int16_t height) {
    bool inside_x = (x >= left) && (x < left + width);
    bool inside_y = (y >= top) && (y < top + height);

    return inside_x && inside_y &&
           ((x == left) || (x == left + width - 1) || (y == top) || (y == top + height - 1));
}

static bool town_area_farm_pixel(int16_t x, int16_t y) {
    bool path = town_area_farm_walkable(x, y);
    bool path_edge = path && (!town_area_farm_walkable((int16_t)(x - 1), y) ||
                              !town_area_farm_walkable((int16_t)(x + 1), y) ||
                              !town_area_farm_walkable(x, (int16_t)(y - 1)) ||
                              !town_area_farm_walkable(x, (int16_t)(y + 1)));
    bool plot = ((x >= 58) && (x < 136)) || ((x >= 184) && (x < 262));
    bool plot_y = ((y >= 50) && (y < 104)) || ((y >= 150) && (y < 222));
    bool plot_edge =
        town_area_rect_edge(x, y, 58, 50, 78, 54) || town_area_rect_edge(x, y, 184, 50, 78, 54) ||
        town_area_rect_edge(x, y, 58, 150, 78, 72) || town_area_rect_edge(x, y, 184, 150, 78, 72);
    bool crop = plot && plot_y && (((x - 4) % 12) == 0) && (((y - 5) % 10) <= 2);
    bool soil_row = plot && plot_y && ((y % 10) == 0) && ((x & 1) == 0);
    bool path_stone = path && ((x % 7) == 0) && ((y % 7) == 0);
    bool grass = !path && !plot_edge && (((x * 3 + y * 5) & 63) == 0);

    return path_edge || path_stone || plot_edge || crop || soil_row || grass;
}

static bool town_area_pond_pixel(int16_t x, int16_t y) {
    bool walkable = town_area_pond_walkable(x, y);
    bool water = (x >= 90) && (x < 230) && (y >= 62) && (y < 134);
    bool bank =
        town_area_rect_edge(x, y, 86, 58, 148, 80) || town_area_rect_edge(x, y, 88, 60, 144, 76);
    bool wave = water && ((y % 12) == 0) && (((x + y / 3) % 18) < 8);
    bool fish = water && (((x + y * 3) % 97) == 0);
    bool path_stone = walkable && ((x % 8) == 0) && ((y % 8) == 0);
    bool grass = !walkable && !water && (((x * 5 + y * 3) & 63) == 0);

    return bank || wave || fish || path_stone || grass;
}

static void town_area_draw_sign(const char *text, int16_t world_x, int16_t world_y,
                                int16_t camera_x, int16_t camera_y, uint8_t width) {
    int16_t screen_x = (int16_t)(world_x - camera_x);
    int16_t screen_y = (int16_t)(world_y - camera_y);

    if ((screen_x >= 0) && (screen_x + width <= (int16_t)GFX_WIDTH) && (screen_y >= 0) &&
        (screen_y + 13 <= (int16_t)GFX_HEIGHT)) {
        gfx_fill_rect((uint8_t)screen_x, (uint8_t)screen_y, width, 13u, false);
        gfx_draw_rect((uint8_t)screen_x, (uint8_t)screen_y, width, 13u, true);
        gfx_draw_text((uint8_t)(screen_x + 4), (uint8_t)(screen_y + 3), text, 1u, true, false);
    }
}

static void town_area_draw_bitmap_clipped(int16_t world_x, int16_t world_y, int16_t camera_x,
                                          int16_t camera_y, const uint8_t *bitmap, uint8_t width,
                                          uint8_t height) {
    for (uint8_t row = 0u; row < height; row++) {
        for (uint8_t column = 0u; column < width; column++) {
            uint8_t data = bitmap[(uint16_t)(row / 8u) * width + column];
            bool on = (data & (uint8_t)(1u << (row & 7u))) != 0u;

            town_area_pixel((int16_t)(world_x + column), (int16_t)(world_y + row), camera_x,
                            camera_y, on);
        }
    }
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void town_area_draw(const town_map_game_t *game, int16_t camera_x, int16_t camera_y) {
    int16_t end_x = (int16_t)(camera_x + GFX_WIDTH);
    int16_t end_y = (int16_t)(camera_y + GFX_HEIGHT);

    for (int16_t y = camera_y; y < end_y; y++) {
        for (int16_t x = camera_x; x < end_x; x++) {
            bool on = (game->scene == TOWN_SCENE_FARM) ? town_area_farm_pixel(x, y)
                                                       : town_area_pond_pixel(x, y);

            if (on) {
                town_area_pixel(x, y, camera_x, camera_y, true);
            }
        }
    }

    if (game->scene == TOWN_SCENE_FARM) {
        town_area_draw_sign("FARM", 140, 222, camera_x, camera_y, 32u);
    } else {
        town_area_draw_sign("POND", 140, 222, camera_x, camera_y, 32u);
    }
}

void town_area_draw_town_signs(int16_t camera_x, int16_t camera_y) {
    town_area_draw_bitmap_clipped(64, 180, camera_x, camera_y, town_farm_entrance_icon,
                                  TOWN_FARM_ICON_WIDTH, TOWN_FARM_ICON_HEIGHT);
    town_area_draw_bitmap_clipped(224, 180, camera_x, camera_y, town_pond_entrance_icon,
                                  TOWN_POND_ICON_WIDTH, TOWN_POND_ICON_HEIGHT);
    town_area_draw_sign("FARM", 64, 168, camera_x, camera_y, 32u);
    town_area_draw_sign("POND", 224, 168, camera_x, camera_y, 32u);
}

void town_area_draw_intro(town_scene_t scene) {
    const uint8_t *bitmap = (scene == TOWN_SCENE_FARM) ? town_farm_bitmap : town_fish_pond_bitmap;
    const char *title = (scene == TOWN_SCENE_FARM) ? "FARM" : "FISH POND";
    uint8_t title_x = (scene == TOWN_SCENE_FARM) ? 52u : 37u;

    gfx_clear();
    gfx_draw_bitmap(16u, 0u, bitmap, TOWN_AREA_BITMAP_WIDTH, TOWN_AREA_BITMAP_HEIGHT, true);
    gfx_draw_text(title_x, 54u, title, 1u, true, false);
}
