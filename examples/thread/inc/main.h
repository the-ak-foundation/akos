#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdbool.h>
#include <stdint.h>

#include "akos.h"
#include "bsp.h"

#ifndef BSP_UART_ENABLED
#define BSP_UART_ENABLED (0u)
#endif

#if BSP_UART_ENABLED
#define EXAMPLE_DEBUG_ENABLED (1u)
#define PRINT_DBG(...)        app_print_dbg(__VA_ARGS__)
#else
#define EXAMPLE_DEBUG_ENABLED (0u)
#define PRINT_DBG(...)        ((void)0)
#endif

#define LOG_BUFFER_SIZE 128u

typedef struct {
    const char* name;
    uint32_t delay_ticks;
} blink_task_ctx_t;

void blink_task(void* p_arg);
#if EXAMPLE_DEBUG_ENABLED
void app_print_dbg(const char* format, ...);
#endif

#endif /* __MAIN_H__ */
