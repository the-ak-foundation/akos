#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "akos.h"
#include "bsp.h"

#define PRINT_DBG(...) app_print_dbg(__VA_ARGS__)

#define TIMER_PERIOD_MS 500u
#define LOG_BUFFER_SIZE 128u

#define TIME_HOURS(ticks)   (((ticks) / 3600000u) % 24u)
#define TIME_MINUTES(ticks) (((ticks) / 60000u) % 60u)
#define TIME_SECONDS(ticks) (((ticks) / 1000u) % 60u)
#define TIME_MILLIS(ticks)  ((ticks) % 1000u)

enum APP_THREAD_ID {
    THREAD_TIMER_DEMO_ID = 0
};

enum APP_MESSAGE_SIGNAL {
    MSG_SIGNAL_TIMER = 1
};

void task_timer_demo(void* p_arg);
void app_print_dbg(const char* format, ...);

#endif /* __MAIN_H__ */
