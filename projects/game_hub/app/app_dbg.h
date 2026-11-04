/**
 ******************************************************************************
 * @file    app_dbg.h
 * @brief   Compile-time application debug and diagnostic prints
 *
 * @module  Game Hub - App
 ******************************************************************************
 */

#ifndef APP_DBG_H
#define APP_DBG_H

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "xprintf.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
/* Use a string literal format, for example APP_DBG("score=%u\n", score).
 * Disabled macros do not evaluate their arguments. Shell output uses xprintf
 * directly and remains available independently of diagnostic logging.
 */
#if defined(GAME_HUB_ENABLE_DEBUG_PRINT) && GAME_HUB_ENABLE_DEBUG_PRINT
#define APP_DBG(...)                                                                               \
    do {                                                                                           \
        xprintf("[DBG] " __VA_ARGS__);                                                             \
    } while (0)
#define APP_PRINT(...)                                                                             \
    do {                                                                                           \
        xprintf("[PRINT] " __VA_ARGS__);                                                           \
    } while (0)
#else
#define APP_DBG(...)                                                                               \
    do {                                                                                           \
    } while (0)
#define APP_PRINT(...)                                                                             \
    do {                                                                                           \
    } while (0)
#endif

#endif /* APP_DBG_H */
