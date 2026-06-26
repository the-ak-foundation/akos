/**
  ******************************************************************************
  * @file    core.h
  * @brief   Kernel control and critical-section API.
  *
  * @author  Snoopy3921 - AK Foundation
  * @date    Created: 2026-06-11
  * @date    Updated: 2026-06-26
  * 
  * @module  AKOS
  ******************************************************************************
  */

#ifndef __CORE_H__
#define __CORE_H__

/* Includes ------------------------------------------------------------------*/
#include "config.h"
#include "log.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if OS_CFG_PRIO_MAX > 255u || OS_CFG_PRIO_MAX < 0u
#error OS_CFG_PRIO_MAX have to be between 0-255
#endif

#define OS_TRUE             ((uint8_t)1)                       /**< Generic AKOS true value. */
#define OS_FALSE            ((uint8_t)0)                       /**< Generic AKOS false value. */

#define core_assert(exp, err)      ((exp) ? (void)0 : LOG_ASSERT("%s", err)) /**< Assert expression and log error string when false. */

/**
 * @brief Enter critical section.
 */
extern void akos_core_enter_critical(void);

/**
 * @brief Exit critical section.
 */
extern void akos_core_exit_critical(void);

/**
 * @brief Initialize kernel subsystems.
 */
extern void akos_core_init(void);

/**
 * @brief Start scheduler and run first thread.
 */
extern void akos_core_run(void);

#define AKOS_CORE_ENTER_CRITICAL()    akos_core_enter_critical() /**< Enter critical section wrapper. */
#define AKOS_CORE_EXIT_CRITICAL()     akos_core_exit_critical()  /**< Exit critical section wrapper. */

#ifdef __cplusplus
}
#endif

#endif /* __CORE_H__ */
