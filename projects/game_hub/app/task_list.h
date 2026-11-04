/**
 ******************************************************************************
 * @file    task_list.h
 * @brief   Task list module
 *
 * @module  Game Hub - App
 ******************************************************************************
 */

#ifndef __GAME_TASK_LIST_H__
#define __GAME_TASK_LIST_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const thread_t game_task_game_hub_desc;
extern const thread_t game_task_life_desc;
extern const thread_t game_task_shell_desc;

#ifdef __cplusplus
}
#endif

#endif /* __GAME_TASK_LIST_H__ */
