#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "console.h"

#define OS_CFG_USE_LOG 0
#define OS_CFG_USE_CLI 0
#define OS_CFG_USER_PRINT(...) console_printf(__VA_ARGS__)

#endif /* APP_CONFIG_H */
