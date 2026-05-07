/*
 * core.h
 *
 *  Created on: Jun 26, 2024
 *      Author: giahu
 *
 *      The kernel provides the system scheduler and thread runtime.
 */

#ifndef CORE_H
#define CORE_H

/**
 * @file core.h
 * @brief Kernel control and critical-section API.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include "config.h"
#include "log.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#if OS_CFG_PRIO_MAX > 255u || OS_CFG_PRIO_MAX < 0u
#error OS_CFG_PRIO_MAX have to be between 0-255
#endif

#define OS_TRUE             ((uint8_t)1)
#define OS_FALSE            ((uint8_t)0)

#define core_assert(exp, err)      ((exp) ? (void)0 : LOG_ASSERT("%s", err))

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
