/**
 ******************************************************************************
 * @file    ufo_bitmap.h
 * @brief   Ufo bitmap module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __UFO_BITMAP_H__
#define __UFO_BITMAP_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t ufo_player[32];
extern const uint8_t ufo_enemy_small[8];
extern const uint8_t ufo_enemy_medium[8];
extern const uint8_t ufo_enemy_big[32];
extern const uint8_t ufo_heart[8];
extern const uint8_t ufo_bomb[8];

#ifdef __cplusplus
}
#endif

#endif /* __UFO_BITMAP_H__ */
