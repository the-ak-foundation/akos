/*!
 * @file	core.h
 * @author	Snoopy3921 - AK Foundation
 * @date	2026/05/08
 * @module	AKOS
 * @brief	Kernel control and critical-section API.
 */

#ifndef CORE_H
#define CORE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "log.h"

#if AKOS_CFG_PRIO_MAX > 255u || AKOS_CFG_PRIO_MAX < 0u
#error AKOS_CFG_PRIO_MAX have to be between 0-255
#endif

#define OS_TRUE                     ((uint8_t)1)
#define OS_FALSE                    ((uint8_t)0)

#define core_assert(exp, err)       ((exp) ? (void)0 : LOG_ASSERT("%s", err))

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

#define AKOS_CORE_ENTER_CRITICAL()    akos_core_enter_critical()
#define AKOS_CORE_EXIT_CRITICAL()     akos_core_exit_critical()

#ifdef __cplusplus
}
#endif
#endif /* CORE_H */
