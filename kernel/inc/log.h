/**
 ******************************************************************************
 * @file    log.h
 * @brief   Logging and assertion macro definitions.
 *
 * @author  Snoopy3921 - AK Foundation
 * @date    Created: 2026-06-11
 * @date    Updated: 2026-06-26
 *
 * @module  AKOS
 ******************************************************************************
 */

#ifndef __LOG_H__
#define __LOG_H__

/* Includes ------------------------------------------------------------------*/
#include "config.h"

#include <stdio.h>

/* Defines -------------------------------------------------------------------*/

/* Terminal color escape sequences. */
#define KNRM            "\x1B[0m"  /**< Reset terminal color. */
#define KRED            "\x1B[31m" /**< Red terminal color. */
#define KGRN            "\x1B[32m" /**< Green terminal color. */
#define KYEL            "\x1B[33m" /**< Yellow terminal color. */
#define KBLU            "\x1B[34m" /**< Blue terminal color. */
#define KMAG            "\x1B[35m" /**< Magenta terminal color. */
#define KCYN            "\x1B[36m" /**< Cyan terminal color. */
#define KWHT            "\x1B[37m" /**< White terminal color. */

/* Per-log-channel enable flags. */
#define LOG_DBG_EN      (1u)       /**< Enable debug log channel. */
#define LOG_WARN_EN     (1u)       /**< Enable warning log channel. */
#define LOG_PRINT_EN    (1u)       /**< Enable print log channel. */
#define LOG_ERROR_EN    (1u)       /**< Enable error log channel. */
#define LOG_ASSERT_EN   (1u)       /**< Enable assert log channel. */
#define LOG_SIG_EN      (1u)       /**< Enable signal log channel. */

#ifndef USER_PRINT
/** Print backend used by logging macros. */
#define USER_PRINT(fmt, ...) OS_CFG_USER_PRINT((fmt), ##__VA_ARGS__)
#endif

#if ((LOG_DBG_EN & OS_CFG_USE_LOG) == 1u)
/** Print debug log. */
#define LOG_DBG(fmt, ...) \
    USER_PRINT(KBLU "[DEBUG: %s:%d] " KYEL fmt KNRM "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
/** Debug log disabled. */
#define LOG_DBG(fmt, ...) ((void)0)
#endif

#if ((LOG_WARN_EN & OS_CFG_USE_LOG) == 1u)
/** Print warning log. */
#define LOG_WARN(fmt, ...) \
    USER_PRINT(KYEL "[WARN: %s:%d] " KNRM fmt KNRM "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
/** Warning log disabled. */
#define LOG_WARN(fmt, ...) ((void)0)
#endif

#if ((LOG_PRINT_EN & OS_CFG_USE_LOG) == 1u)
/** Print normal log. */
#define LOG_PRINT(fmt, ...) USER_PRINT("[PRINTLN] " fmt "\r\n", ##__VA_ARGS__)
#else
/** Print log disabled. */
#define LOG_PRINT(fmt, ...) ((void)0)
#endif

#if ((LOG_ERROR_EN & OS_CFG_USE_LOG) == 1u)
/** Print error log. */
#define LOG_ERROR(fmt, ...) \
    USER_PRINT(KRED "[ERROR: %s:%d] " fmt KNRM "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
/** Error log disabled. */
#define LOG_ERROR(fmt, ...) ((void)0)
#endif

#if ((LOG_ASSERT_EN & OS_CFG_USE_LOG) == 1u)
/** Print assertion failure log. */
#define LOG_ASSERT(fmt, ...) \
    USER_PRINT(KRED "[ASSERT FAILED: %s:%d] " fmt KNRM "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
/** Assert log disabled. */
#define LOG_ASSERT(fmt, ...) ((void)0)
#endif

#if ((LOG_SIG_EN & OS_CFG_USE_LOG) == 1u)
/** Print signal log. */
#define LOG_SIG(fmt, ...) \
    USER_PRINT(KGRN "[SIGNAL: %s:%d] " KMAG fmt KNRM "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
/** Signal log disabled. */
#define LOG_SIG(fmt, ...) ((void)0)
#endif

#endif /* __LOG_H__ */
