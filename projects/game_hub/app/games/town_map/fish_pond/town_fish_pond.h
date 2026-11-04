/**
 ******************************************************************************
 * @file    town_fish_pond.h
 * @brief   Town fish pond module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __TOWN_FISH_POND_H__
#define __TOWN_FISH_POND_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdint.h>

#include "town_map_game.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void town_fish_pond_update(town_map_game_t *game, uint8_t input_state, uint8_t pressed,
                           uint32_t elapsed_ms);
void town_fish_pond_draw(const town_map_game_t *game);

#ifdef __cplusplus
}
#endif

#endif /* __TOWN_FISH_POND_H__ */
