/**
 ******************************************************************************
 * @file    town_farm.h
 * @brief   Town farm module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __TOWN_FARM_H__
#define __TOWN_FARM_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdint.h>

#include "town_map_game.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define TOWN_FARM_PLOT_COUNT   (10u)
#define TOWN_FARM_CANCEL_INDEX (5u)

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void town_farm_update(town_map_game_t *game, uint8_t input_state, uint8_t pressed,
                      uint32_t elapsed_ms);
void town_farm_draw(const town_map_game_t *game);
void town_farm_draw_coin_counter(const town_map_game_t *game, uint8_t x, uint8_t y);

#ifdef __cplusplus
}
#endif

#endif /* __TOWN_FARM_H__ */
