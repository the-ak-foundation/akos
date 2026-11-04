/**
 ******************************************************************************
 * @file    app.h
 * @brief   App module
 *
 * @module  Game Hub - App
 ******************************************************************************
 */

#ifndef __GAME_APP_H__
#define __GAME_APP_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdint.h>

#include "akos.h"
#include "game_input.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define GAME_FRAME_PERIOD_TICKS   (16u)
#define GAME_DISPLAY_PERIOD_TICKS (33u)
#define GAME_TASK_STACK_SIZE      (256u)
#define GAME_LIFE_STACK_SIZE      (64u)
#define GAME_SHELL_STACK_SIZE     (256u)

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef enum {
    GAME_THREAD_LOGIC = 0,
    GAME_THREAD_LIFE,
    GAME_THREAD_SHELL,
    GAME_THREAD_COUNT
} game_thread_id_t;

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void task_game_hub(void *argument);
void task_life(void *argument);
void game_app_display_pause(void);
void game_app_display_resume(void);

#ifdef __cplusplus
}
#endif

#endif /* __GAME_APP_H__ */
