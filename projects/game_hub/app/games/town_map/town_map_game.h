/**
 ******************************************************************************
 * @file    town_map_game.h
 * @brief   Town map game module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __TOWN_MAP_GAME_H__
#define __TOWN_MAP_GAME_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define TOWN_WORLD_WIDTH   (320)
#define TOWN_WORLD_HEIGHT  (256)
#define TOWN_ROAD_WIDTH    (32)
#define TOWN_PLAYER_WIDTH  (16)
#define TOWN_PLAYER_HEIGHT (16)

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef enum {
    TOWN_DIR_UP = 0,
    TOWN_DIR_RIGHT,
    TOWN_DIR_DOWN,
    TOWN_DIR_LEFT,
} town_direction_t;

typedef enum {
    TOWN_PLAYER_STANDING = 0,
    TOWN_PLAYER_WALKING,
} town_player_state_t;

typedef enum {
    TOWN_SCENE_TOWN = 0,
    TOWN_SCENE_FARM,
    TOWN_SCENE_FISH_POND,
} town_scene_t;

typedef enum {
    TOWN_CHARACTER_SOCCER = 0,
    TOWN_CHARACTER_NEW,
    TOWN_CHARACTER_COUNT,
} town_character_t;

typedef struct {
    int16_t x;
    int16_t y;
    town_direction_t direction;
    town_player_state_t state;
    uint8_t animation_frame;
} town_player_t;

typedef struct {
    town_player_t player;
    uint32_t move_accumulator_ms;
    uint32_t animation_accumulator_ms;
    uint32_t mode_hold_ms;
    uint32_t farm_clock_accumulator_ms;
    uint32_t farm_random_state;
    uint16_t area_intro_ms;
    uint16_t farm_minutes;
    uint16_t farm_growth_ms[10];
    uint8_t farm_plots[10];
    uint8_t farm_crop_types[10];
    uint8_t fish_owned[8];
    uint8_t farm_selected_plot;
    uint8_t farm_selected_tool;
    uint8_t farm_focus;
    uint8_t coin_count;
    uint8_t fish_selected;
    uint8_t selected_character;
    uint8_t input_previous;
    town_scene_t scene;
    town_character_t character;
    bool selecting_character;
    bool return_to_menu;
} town_map_game_t;

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
bool town_world_player_fits(int16_t x, int16_t y);
int16_t town_camera_x(const town_map_game_t *game);
int16_t town_camera_y(const town_map_game_t *game);

void town_map_game_init(void *context, uint32_t seed);
void town_map_game_deinit(void *context);
void town_map_game_update(void *context, uint8_t input_state, uint32_t elapsed_ms);
void town_map_game_draw(const void *context);

#ifdef __cplusplus
}
#endif

#endif /* __TOWN_MAP_GAME_H__ */
