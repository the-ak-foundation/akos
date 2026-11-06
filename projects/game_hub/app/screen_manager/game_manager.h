/**
 ******************************************************************************
 * @file    game_manager.h
 * @brief   Game manager module
 *
 * @module  Game Hub - Screen Manager
 ******************************************************************************
 */

#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "game_registry.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define GAME_MANAGER_CONTEXT_SIZE          (512u)
#define GAME_MANAGER_REPEAT_DELAY_UPDATES  (18u)
#define GAME_MANAGER_REPEAT_PERIOD_UPDATES (6u)
#define GAME_MANAGER_AUTO_SNAKE_IDLE_MS    (30000u)

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef enum {
    GAME_MANAGER_MENU = 0,
    GAME_MANAGER_RUNNING,
} game_manager_mode_t;

typedef union {
    uint32_t alignment;
    uint8_t bytes[GAME_MANAGER_CONTEXT_SIZE];
} game_context_storage_t;

typedef struct {
    game_context_storage_t context;
    const game_entry_t *active_game;
    uint8_t selected_index;
    uint8_t input_previous;
    uint8_t navigation_hold_updates;
    uint32_t uptime_ms;
    uint32_t menu_idle_ms;
    bool input_locked;
    game_manager_mode_t mode;
} game_manager_t;

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void game_manager_init(game_manager_t *manager);
void game_manager_update(game_manager_t *manager, uint8_t input_state, uint32_t elapsed_ms);
void game_manager_draw(const game_manager_t *manager);
uint8_t game_manager_take_events(game_manager_t *manager);

#ifdef __cplusplus
}
#endif

#endif /* __GAME_MANAGER_H__ */
