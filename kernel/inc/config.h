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
extern "C"
{
#endif

/* ============================================================
 * 1. Kernel
 * ============================================================ */

#ifndef OS_CFG_SYSTICK_CLOCK_HZ
#define OS_CFG_SYSTICK_CLOCK_HZ           ((uint32_t)48000000u)    /**< SysTick input clock frequency in Hz. */
#endif

#ifndef OS_CFG_HEAP_SIZE
#define OS_CFG_HEAP_SIZE                  ((size_t)2144u)          /**< Total static heap size in bytes for OS allocator. */
#endif

#ifndef OS_CFG_PRIO_MAX
#define OS_CFG_PRIO_MAX                   (10u)                    /**< Number of priority levels (0 .. OS_CFG_PRIO_MAX-1). */
#endif

#ifndef OS_CFG_DELAY_MAX
#define OS_CFG_DELAY_MAX                  ((uint32_t)0xffffffffUL) /**< Maximum delay timeout value (effectively infinite wait). */
#endif

/* ============================================================
 * 2. Tasks
 * ============================================================ */

#ifndef OS_CFG_TASK_STK_SIZE_MIN
#define OS_CFG_TASK_STK_SIZE_MIN          ((size_t)17u)         /**< Minimum task stack size in 32-bit words (must be > 64 bytes). */
#endif

#ifndef OS_CFG_TASK_STACK_FILL_BYTE
#define OS_CFG_TASK_STACK_FILL_BYTE       (0x5Au)               /**< Fill byte used to initialize task stacks for debug/usage checks. */
#endif

#ifndef OS_CFG_TASK_MSG_Q_SIZE_NORMAL
#define OS_CFG_TASK_MSG_Q_SIZE_NORMAL     (8u)                  /**< Default per-task message queue depth. */
#endif

/* ============================================================
 * 3. Messages
 * ============================================================ */

#ifndef OS_CFG_MSG_POOL_SIZE
#define OS_CFG_MSG_POOL_SIZE              (32u)                 /**< Number of preallocated message objects in global pool. */
#endif

/* ============================================================
 * 4. Timers
 * ============================================================ */

#ifndef OS_CFG_TIMER_POOL_SIZE
#define OS_CFG_TIMER_POOL_SIZE            (8u)                  /**< Maximum number of software timers allocatable at runtime. */
#endif

#ifndef OS_CFG_TIMER_TASK_PRI
#define OS_CFG_TIMER_TASK_PRI             (0u)                  /**< Priority for internal timer task (lower value means higher priority). */
#endif

/* ============================================================
 * 5. Optional Features
 * ============================================================ */

#ifndef OS_CFG_USE_LOG
#define OS_CFG_USE_LOG                    (1u)                  /**< Enable (1) or disable (0) logging helpers. */
#endif

#ifndef OS_CFG_USER_PRINT
#define OS_CFG_USER_PRINT(...)            printf(__VA_ARGS__)   /**< User-supplied print backend hook. */
#endif

#ifndef OS_CFG_USE_CLI
#define OS_CFG_USE_CLI                    (0u)                  /**< Enable (1) or disable (0) CLI support hooks. */
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CONFIG_H__ */
