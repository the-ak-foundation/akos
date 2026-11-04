/**
 ******************************************************************************
 * @file    block_stack_game.h
 * @brief   Block stack game module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __BLOCK_STACK_GAME_H__
#define __BLOCK_STACK_GAME_H__

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

#define BLOCK_BOARD_WIDTH  (10u)
#define BLOCK_BOARD_HEIGHT (15u)
#define BLOCK_PIECE_COUNT  (7u)

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef enum {
    BLOCK_PIECE_I = 0,
    BLOCK_PIECE_O,
    BLOCK_PIECE_T,
    BLOCK_PIECE_S,
    BLOCK_PIECE_Z,
    BLOCK_PIECE_J,
    BLOCK_PIECE_L,
} block_piece_type_t;

typedef struct {
    int8_t x;
    int8_t y;
    uint8_t type;
    uint8_t rotation;
} block_piece_t;

typedef struct {
    uint16_t board[BLOCK_BOARD_HEIGHT];
    uint32_t random_state;
    uint32_t score;
    uint32_t fall_accumulator_ms;
    uint16_t fall_interval_ms;
    uint16_t lines_cleared;
    block_piece_t current_piece;
    uint8_t next_piece;
    uint8_t level;
    uint8_t input_previous;
    bool game_over;
    bool return_to_menu;
} block_stack_game_t;

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void block_stack_game_init(void *context, uint32_t seed);
void block_stack_game_deinit(void *context);
void block_stack_game_update(void *context, uint8_t input_state, uint32_t elapsed_ms);
void block_stack_game_draw(const void *context);

bool block_stack_piece_fits(const block_stack_game_t *game, const block_piece_t *piece);
uint8_t block_stack_clear_lines(block_stack_game_t *game);

#ifdef __cplusplus
}
#endif

#endif /* __BLOCK_STACK_GAME_H__ */
