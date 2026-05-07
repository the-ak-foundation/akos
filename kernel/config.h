/*
 * config.h
 *
 *  Created on: Jun 26, 2024
 *      Author: giahu
 */
/**
 * @file config.h
 * @brief Build-time configuration macros for AK-mOS.
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

/* Kernel common config */
#define OS_CFG_SYSTICK_CLOCK_HZ           ((uint32_t)32000000)
#define OS_CFG_HEAP_SIZE                  ((size_t)2144U)      /**< Total static heap size in bytes for OS allocator. */
#define OS_CFG_PRIO_MAX                   (10)                     /**< Number of priority levels (0 .. OS_CFG_PRIO_MAX-1). */
#define OS_CFG_DELAY_MAX                  ((uint32_t)0xffffffffUL) /**< Maximum delay timeout value (effectively infinite wait). */

/* Task config */
#define OS_CFG_TASK_STK_SIZE_MIN          ((size_t)17u) /**< Minimum task stack size in 32-bit words (must be > 64 bytes). */
#define OS_CFG_TASK_STACK_FILL_BYTE       (0x5Au)       /**< Fill byte used to initialize task stacks for debug/usage checks. */
#define OS_CFG_TASK_MSG_Q_SIZE_NORMAL     (8u)          /**< Default per-task message queue depth. */

/* Messages config */
#define OS_CFG_MSG_POOL_SIZE              (32u) /**< Number of preallocated message objects in global pool. */

/* Timers config */
#define OS_CFG_TIMER_POOL_SIZE            (8u) /**< Maximum number of software timers allocatable at runtime. */
#define OS_CFG_TIMER_TASK_PRI             (0u) /**< Priority for internal timer task (lower value means higher priority). */

/* Log config */
#ifndef OS_CFG_USE_LOG
#define OS_CFG_USE_LOG                    (1u) /**< Enable (`1`) or disable (`0`) logging helpers. */
#endif

#ifndef OS_CFG_USER_PRINT
#define OS_CFG_USER_PRINT(...)            printf(__VA_ARGS__) /**< User-supplied print backend hook. */
#endif

/* Command line interface config */
#ifndef OS_CFG_USE_CLI
#define OS_CFG_USE_CLI                    (0u) /**< Enable (`1`) or disable (`0`) CLI support hooks. */
#endif

#ifdef __cplusplus
}
#endif
#endif /* CONFIG_H */
