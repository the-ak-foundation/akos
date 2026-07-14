#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdbool.h>
#include <stdint.h>

#include "akos.h"
#include "bsp.h"

#define PRINT_DBG(...) app_print_dbg(__VA_ARGS__)

#define LOG_BUFFER_SIZE 128u

typedef struct {
    const char* name;
    uint32_t delay_ticks;
} blink_task_ctx_t;

void blink_task(void* p_arg);
void app_print_dbg(const char* format, ...);

#endif /* __MAIN_H__ */
