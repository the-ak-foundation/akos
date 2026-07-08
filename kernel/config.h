/*!
 * @file	config.h
 * @author	Snoopy3921 - AK Foundation
 * @date	2026/05/08
 * @module	AKOS
 * @brief	Build-time configuration macros for AKOS.
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stddef.h>
#include <stdint.h>

/* Kernel common config */
#define AKOS_CFG_SYSTICK_CLOCK_HZ           ((uint32_t)32000000)       /**< SysTick input clock frequency in Hz. */

#ifndef AKOS_CFG_HEAP_SIZE
#define AKOS_CFG_HEAP_SIZE                  ((size_t)8192U)             /**< Total static heap size in bytes for OS allocator. */
#endif

#define AKOS_CFG_PRIO_MAX                   (10)                        /**< Number of priority levels (0 .. AKOS_CFG_PRIO_MAX-1). */

#define AKOS_CFG_DELAY_MAX                  ((uint32_t)0xffffffffUL)    /**< Maximum delay timeout value (effectively infinite wait). */

/* Task config */
#define AKOS_CFG_TASK_STK_SIZE_MIN          ((size_t)17u)               /**< Minimum task stack size in 32-bit words (must be > 64 bytes). */

#define AKOS_CFG_TASK_STACK_FILL_BYTE       (0x5Au)                     /**< Fill byte used to initialize task stacks for debug/usage checks. */

#define AKOS_CFG_TASK_MSG_Q_SIZE_NORMAL     (8u)                        /**< Default per-task message queue depth. */

/* Messages config */
#ifndef AKOS_CFG_MSG_POOL_SIZE
#define AKOS_CFG_MSG_POOL_SIZE              (32u)                       /**< Number of preallocated message objects in global pool. */
#endif

/* Timers config */
#ifndef AKOS_CFG_TIMER_POOL_SIZE
#define AKOS_CFG_TIMER_POOL_SIZE            (8u)                        /**< Maximum number of software timers allocatable at runtime. */
#endif

#define AKOS_CFG_TIMER_TASK_PRI             (0u)                        /**< Priority for internal timer task (lower value means higher priority). */

/* Shell config */
#ifndef AKOS_CFG_USE_SHELL
#define AKOS_CFG_USE_SHELL                  (1u)                        /**< Enable (`1`) or disable (`0`) the built-in AKOS shell thread. */
#endif

#define AKOS_CFG_SHELL_PROMPT_NAME          "akos:~$ "                  /**< Prompt name for the shell. */
#define AKOS_CFG_USE_RUNTIME_STATS          (1u)                        /**< Enable (`1`) or disable (`0`) runtime statistics and CPU load calculation. */
#define AKOS_CFG_SHELL_TASK_PRI             (1u)                        /**< Priority for the internal shell task. */
#define AKOS_CFG_SHELL_TASK_MSG_Q_SIZE      (8u)                        /**< Message queue depth for the internal shell task. */
#define AKOS_CFG_SHELL_TASK_STK_SIZE        (192u)                      /**< Stack size in 32-bit words for the internal shell task. */
#define AKOS_CFG_SHELL_RX_BUFFER_SIZE       (128u)                      /**< UART RX ring buffer size in bytes. */
#define AKOS_CFG_SHELL_LINE_BUFFER_SIZE     (64u)                       /**< Maximum command line length. */
#define AKOS_CFG_SHELL_MAX_ARGS             (8u)                        /**< Maximum number of parsed arguments per command line. */

/* Log config */
#ifndef AKOS_CFG_USE_LOG
#define AKOS_CFG_USE_LOG                    (1u)                        /**< Enable (`1`) or disable (`0`) logging helpers. */
#endif

/* Command line interface config */
#ifndef AKOS_CFG_USE_CLI
#define AKOS_CFG_USE_CLI                    (0u)                        /**< Enable (`1`) or disable (`0`) CLI support hooks. */
#endif

#ifdef __cplusplus
}
#endif
#endif /* CONFIG_H */
