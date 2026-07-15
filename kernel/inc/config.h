/**
 ******************************************************************************
 * @file    config.h
 * @brief   Build-time configuration macros for AKOS.
 *
 * @author  HUYNNG | BUUPQ
 * @date    Created: 2026-06-11
 * @date    Updated: 2026-06-26
 *
 * @module  AKOS
 ******************************************************************************
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 1. Kernel
 * ============================================================ */

#ifndef OS_CFG_SYSTICK_CLOCK_HZ
/** SysTick input clock frequency in Hz. */
#define OS_CFG_SYSTICK_CLOCK_HZ       ((uint32_t)48000000u)
#endif

#ifndef OS_CFG_HEAP_SIZE
/** Total static heap size in bytes for OS allocator. */
#define OS_CFG_HEAP_SIZE              ((size_t)2144u)
#endif

#ifndef OS_CFG_PRIO_MAX
/** Number of priority levels (0 .. OS_CFG_PRIO_MAX-1). */
#define OS_CFG_PRIO_MAX               (10u)
#endif

#ifndef OS_CFG_DELAY_MAX
/** Maximum delay timeout value (effectively infinite wait). */
#define OS_CFG_DELAY_MAX              ((uint32_t)0xffffffffUL)
#endif

/* ============================================================
 * 2. Tasks
 * ============================================================ */

#ifndef OS_CFG_TASK_STK_SIZE_MIN
/** Minimum task stack size in 32-bit words (must be > 64 bytes). */
#define OS_CFG_TASK_STK_SIZE_MIN      ((size_t)17u)
#endif

#ifndef OS_CFG_TASK_STACK_FILL_BYTE
/** Fill byte used to initialize task stacks for debug/usage checks. */
#define OS_CFG_TASK_STACK_FILL_BYTE   (0x5Au)
#endif

#ifndef OS_CFG_TASK_MSG_Q_SIZE_NORMAL
/** Default per-task message queue depth. */
#define OS_CFG_TASK_MSG_Q_SIZE_NORMAL (8u)
#endif

/* ============================================================
 * 3. Messages
 * ============================================================ */

#ifndef OS_CFG_MSG_POOL_SIZE
/** Number of preallocated message objects in global pool. */
#define OS_CFG_MSG_POOL_SIZE          (32u)
#endif

/* ============================================================
 * 4. Timers
 * ============================================================ */

#ifndef OS_CFG_TIMER_POOL_SIZE
/** Maximum number of software timers allocatable at runtime. */
#define OS_CFG_TIMER_POOL_SIZE        (8u)
#endif

#ifndef OS_CFG_TIMER_TASK_PRI
/** Priority for internal timer task (lower value means higher priority). */
#define OS_CFG_TIMER_TASK_PRI         (0u)
#endif

/* ============================================================
 * 5. Optional Features
 * ============================================================ */

#ifndef OS_CFG_USE_LOG
/** Enable (1) or disable (0) logging helpers. */
#define OS_CFG_USE_LOG                (1u)
#endif

#ifndef OS_CFG_USER_PRINT
/** User-supplied print backend hook. */
#define OS_CFG_USER_PRINT(...)        printf(__VA_ARGS__)
#endif

#ifndef OS_CFG_USE_CLI
/** Enable (1) or disable (0) CLI support hooks. */
#define OS_CFG_USE_CLI                (0u)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CONFIG_H__ */
