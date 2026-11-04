/**
 ******************************************************************************
 * @file    game_input.h
 * @brief   Game input
 *
 * @module  Game Hub - Input
 ******************************************************************************
 */

#ifndef GAME_INPUT_H
#define GAME_INPUT_H

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
enum {
    GAME_INPUT_UP = 1u << 0,
    GAME_INPUT_DOWN = 1u << 1,
    GAME_INPUT_MODE = 1u << 2,
};

/* Read the board buttons as logical game input flags. */
uint8_t game_input_read(void);

#ifdef __cplusplus
}
#endif

#endif
