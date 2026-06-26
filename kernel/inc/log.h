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
#define USER_PRINT(fmt, ...) OS_CFG_USER_PRINT((const char *)fmt, ##__VA_ARGS__) /**< Print backend used by logging macros. */
#endif

#if ( LOG_DBG_EN & OS_CFG_USE_LOG ) == 1
#define LOG_DBG(fmt, ...) USER_PRINT(KBLU "[DEBUG: %s:%d] " KYEL fmt KNRM "\r\n", (uint8_t *)__FILE__, __LINE__, ##__VA_ARGS__) /**< Print debug log. */
#else
#define LOG_DBG(fmt, ...) ((void *)0) /**< Debug log disabled. */
#endif

#if ( LOG_WARN_EN & OS_CFG_USE_LOG ) == 1
#define LOG_WARN(fmt, ...) USER_PRINT(KYEL "[WARN: %s:%d] " KNRM fmt KNRM "\r\n", (uint8_t *)__FILE__, __LINE__, ##__VA_ARGS__) /**< Print warning log. */
#else
#define LOG_WARN(fmt, ...) ((void *)0) /**< Warning log disabled. */
#endif

#if ( LOG_PRINT_EN & OS_CFG_USE_LOG ) == 1
#define LOG_PRINT(fmt, ...) USER_PRINT("[PRINTLN] " fmt "\r\n", ##__VA_ARGS__) /**< Print normal log. */
#else
#define LOG_PRINT(fmt, ...) ((void *)0) /**< Print log disabled. */
#endif

#if ( LOG_ERROR_EN & OS_CFG_USE_LOG ) == 1
#define LOG_ERROR(fmt, ...) USER_PRINT(KRED "[ERROR: %s:%d] " fmt KNRM "\r\n", (uint8_t *)__FILE__, __LINE__, ##__VA_ARGS__) /**< Print error log. */
#else
#define LOG_ERROR(fmt, ...) ((void *)0) /**< Error log disabled. */
#endif

#if ( LOG_ASSERT_EN & OS_CFG_USE_LOG ) == 1
#define LOG_ASSERT(fmt, ...) USER_PRINT(KRED "[ASSERT FAILED: %s:%d] " fmt KNRM "\r\n", (uint8_t *)__FILE__, __LINE__, ##__VA_ARGS__) /**< Print assert failure log. */
#else
#define LOG_ASSERT(fmt, ...) ((void *)0) /**< Assert log disabled. */
#endif

#if ( LOG_SIG_EN & OS_CFG_USE_LOG ) == 1
#define LOG_SIG(fmt, ...) USER_PRINT(KGRN "[SIGNAL: %s:%d] " KMAG fmt KNRM "\r\n", (uint8_t *)__FILE__, __LINE__, ##__VA_ARGS__) /**< Print signal log. */
#else
#define LOG_SIG(fmt, ...) ((void *)0) /**< Signal log disabled. */
#endif

#endif /* __LOG_H__ */
