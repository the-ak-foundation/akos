/**
 ******************************************************************************
 * @file    hollow_game.h
 * @brief   Hollow game module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __HOLLOW_GAME_H__
#define __HOLLOW_GAME_H__

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

#define HOLLOW_COLUMN_COUNT    (18u)
#define HOLLOW_CAVE_WIDTH      (144u)
#define HOLLOW_RENDER_Y_OFFSET (-5)

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef enum {
    HOLLOW_EVENT_NONE = 0u,
    HOLLOW_EVENT_START = (1u << 0),
    HOLLOW_EVENT_MOVE = (1u << 1),
    HOLLOW_EVENT_CRUSH = (1u << 2),
    HOLLOW_EVENT_GAME_OVER = (1u << 3),
} hollow_event_t;

typedef struct {
    uint8_t top;
    uint8_t bottom;
} hollow_column_t;

typedef struct {
    uint8_t time;
    int8_t base;
} hollow_debris_t;

typedef struct {
    hollow_column_t columns[HOLLOW_COLUMN_COUNT];
    hollow_debris_t debris[HOLLOW_CAVE_WIDTH];
    uint32_t random_state;
    uint16_t score;
    uint16_t counter;
    uint16_t cave_phase;
    uint16_t cave_loop_count;
    int16_t cave_gap;
    int8_t cave_base_top;
    int8_t cave_base_bottom;
    int8_t cave_hollow_count;
    int8_t player_jump;
    uint8_t cave_offset;
    uint8_t cave_max_gap;
    uint8_t player_x;
    uint8_t player_column;
    uint8_t player_move;
    uint8_t input_previous;
    uint8_t events;
    bool player_direction;
    bool starting;
    bool game_over;
    bool return_to_menu;
} hollow_game_t;

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void hollow_game_init(hollow_game_t *game);
void hollow_game_update(hollow_game_t *game, uint8_t input_state);
void hollow_game_draw(const hollow_game_t *game);
uint8_t hollow_game_take_events(hollow_game_t *game);

#ifdef __cplusplus
}
#endif

#endif /* __HOLLOW_GAME_H__ */
