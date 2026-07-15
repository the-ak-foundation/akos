#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdbool.h>
#include <stddef.h>
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

#define HIGH_TASK_PERIOD_MS 100u
#define LOW_TASK_PERIOD_MS  1000u
#define LOG_BUFFER_SIZE     128u

#define TIME_HOURS(ticks)   (((ticks) / 3600000u) % 24u)
#define TIME_MINUTES(ticks) (((ticks) / 60000u) % 60u)
#define TIME_SECONDS(ticks) (((ticks) / 1000u) % 60u)
#define TIME_MILLIS(ticks)  ((ticks) % 1000u)

enum APP_THREAD_ID {
    THREAD_PRIO_HIGH_ID = 0,
    THREAD_PRIO_LOW_ID
};

enum APP_MESSAGE_SIGNAL {
    MSG_SIGNAL_PRIO_HIGH = 1,
    MSG_SIGNAL_PRIO_LOW
};

void task_prio_high(void* p_arg);
void task_prio_low(void* p_arg);
#if EXAMPLE_DEBUG_ENABLED
void app_print_dbg(const char* format, ...);
#endif

#endif /* __MAIN_H__ */
