/**
 ******************************************************************************
 * @file    town_areas.h
 * @brief   Town areas module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __TOWN_AREAS_H__
#define __TOWN_AREAS_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>
#include <stdint.h>

#include "town_map_game.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define TOWN_BRANCH_WIDTH (24)

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
bool town_area_town_point_on_road(int16_t x, int16_t y);
bool town_area_player_fits(town_scene_t scene, int16_t x, int16_t y);
bool town_area_check_transition(town_map_game_t *game);
void town_area_draw(const town_map_game_t *game, int16_t camera_x, int16_t camera_y);
void town_area_draw_town_signs(int16_t camera_x, int16_t camera_y);
void town_area_draw_intro(town_scene_t scene);

void town_area_enter_farm(town_map_game_t *game);
void town_area_enter_fish_pond(town_map_game_t *game);
void town_area_leave_farm(town_map_game_t *game);
void town_area_leave_fish_pond(town_map_game_t *game);

#ifdef __cplusplus
}
#endif

#endif /* __TOWN_AREAS_H__ */
