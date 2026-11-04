/**
 ******************************************************************************
 * @file    bitmap_viewer_game.h
 * @brief   Bitmap viewer game module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __BITMAP_VIEWER_GAME_H__
#define __BITMAP_VIEWER_GAME_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef struct {
    uint8_t selected;
    uint8_t input_previous;
    bool return_to_menu;
} bitmap_viewer_game_t;

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void bitmap_viewer_game_init(void *context, uint32_t seed);
void bitmap_viewer_game_deinit(void *context);
void bitmap_viewer_game_update(void *context, uint8_t input_state, uint32_t elapsed_ms);
void bitmap_viewer_game_draw(const void *context);

#ifdef __cplusplus
}
#endif

#endif /* __BITMAP_VIEWER_GAME_H__ */
