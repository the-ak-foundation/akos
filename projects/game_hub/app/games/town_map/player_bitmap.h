/**
 ******************************************************************************
 * @file    player_bitmap.h
 * @brief   Player bitmap module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __PLAYER_BITMAP_H__
#define __PLAYER_BITMAP_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define PLAYER_SPRITE_WIDTH        (16u)
#define PLAYER_SPRITE_HEIGHT       (16u)
#define PLAYER_SPRITE_BYTES        (32u)
#define PLAYER_FRAME_PER_DIRECTION (2u)
#define PLAYER_SPRITE_COUNT        (8u)

/*
 * Bitmap format:
 * - 16 rows.
 * - 2 bytes per row.
 * - MSB is the left-most pixel.
 * - Bit 1: OLED pixel on; bit 0: OLED pixel off.
 */

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef enum {
    PLAYER_DIR_DOWN = 0,
    PLAYER_DIR_LEFT,
    PLAYER_DIR_RIGHT,
    PLAYER_DIR_UP,
    PLAYER_DIR_COUNT
} PlayerDirection;

typedef enum { PLAYER_FRAME_0 = 0, PLAYER_FRAME_1, PLAYER_FRAME_COUNT } PlayerFrame;

typedef enum {
    PLAYER_SPRITE_DOWN_0 = 0,
    PLAYER_SPRITE_DOWN_1,
    PLAYER_SPRITE_LEFT_0,
    PLAYER_SPRITE_LEFT_1,
    PLAYER_SPRITE_RIGHT_0,
    PLAYER_SPRITE_RIGHT_1,
    PLAYER_SPRITE_UP_0,
    PLAYER_SPRITE_UP_1
} PlayerSpriteId;

#define PLAYER_SPRITE_INDEX(direction, frame)                                                      \
    ((uint8_t)((uint8_t)(direction) * PLAYER_FRAME_PER_DIRECTION + (uint8_t)(frame)))

extern const uint8_t g_player_bitmap[PLAYER_SPRITE_COUNT][PLAYER_SPRITE_BYTES];

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
const uint8_t *player_get_bitmap(PlayerDirection direction, PlayerFrame frame);

#ifdef __cplusplus
}
#endif

#endif /* __PLAYER_BITMAP_H__ */
