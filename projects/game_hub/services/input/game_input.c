/**
 ******************************************************************************
 * @file    game_input.c
 * @brief   Map board buttons to logical game input flags
 *
 * @module  Game Hub - Input
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "game_input.h"

#include "bsp.h"

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
uint8_t game_input_read(void) {
    uint8_t input_state = 0u;

    if (bsp_button_is_pressed(BSP_BUTTON_UP)) {
        input_state |= GAME_INPUT_UP;
    }
    if (bsp_button_is_pressed(BSP_BUTTON_DOWN)) {
        input_state |= GAME_INPUT_DOWN;
    }
    if (bsp_button_is_pressed(BSP_BUTTON_MODE)) {
        input_state |= GAME_INPUT_MODE;
    }

    return input_state;
}
