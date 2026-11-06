/**
 ******************************************************************************
 * @file    task_list.c
 * @brief   Task list module
 *
 * @module  Game Hub - App
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "task_list.h"

#include <stddef.h>

#include "app.h"
#include "task_shell.h"

AKOS_THREAD_DEFINE(game_task_game_hub_desc, GAME_THREAD_LOGIC, task_game_hub, NULL, 1u, 0u,
                   GAME_TASK_STACK_SIZE);
AKOS_THREAD_DEFINE(game_task_life_desc, GAME_THREAD_LIFE, task_life, NULL, 2u, 0u,
                   GAME_LIFE_STACK_SIZE);
AKOS_THREAD_DEFINE(game_task_shell_desc, GAME_THREAD_SHELL, task_shell, NULL, 3u, 0u,
                   GAME_SHELL_STACK_SIZE);
