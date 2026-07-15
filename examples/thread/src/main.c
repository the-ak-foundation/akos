#include "main.h"

#if EXAMPLE_DEBUG_ENABLED
#include <stdarg.h>
#include <stdio.h>
#endif

#define NOTE "MAIN"

static const blink_task_ctx_t blink_100ms_ctx = {
    .name        = "BLINK 100MS",
    .delay_ticks = 100u,
};

static const blink_task_ctx_t blink_200ms_ctx = {
    .name        = "BLINK 200MS",
    .delay_ticks = 200u,
};

static const blink_task_ctx_t blink_400ms_ctx = {
    .name        = "BLINK 400MS",
    .delay_ticks = 400u,
};

AKOS_THREAD_DEFINE(blink_100ms, 0u, blink_task, &blink_100ms_ctx, 4u, 0u,
                   THREAD_TASK_STACK_SIZE);
AKOS_THREAD_DEFINE(blink_200ms, 1u, blink_task, &blink_200ms_ctx, 5u, 0u,
                   THREAD_TASK_STACK_SIZE);
AKOS_THREAD_DEFINE(blink_400ms, 2u, blink_task, &blink_400ms_ctx, 6u, 0u,
                   THREAD_TASK_STACK_SIZE);

#if EXAMPLE_DEBUG_ENABLED
void app_print_dbg(const char* format, ...) {
    char log_buffer[LOG_BUFFER_SIZE];
    va_list arguments;
    int result;

    va_start(arguments, format);
    result = vsnprintf(log_buffer, sizeof(log_buffer), format, arguments);
    va_end(arguments);

    if ((result >= 0) && ((size_t)result < sizeof(log_buffer))) {
        bsp_uart_puts(log_buffer);
    }
}
#endif

int main(void) {
    bsp_init();

    PRINT_DBG("\r\n");
    PRINT_DBG("    _    _  __  ___  ____  \r\n");
    PRINT_DBG("   / \\  | |/ / / _ \\/ ___| \r\n");
    PRINT_DBG("  / _ \\ | ' / | | | \\___ \\ \r\n");
    PRINT_DBG(" / ___ \\| . \\ | |_| |___) |\r\n");
    PRINT_DBG("/_/   \\_\\_|\\_\\ \\___/|____/ \r\n");
    PRINT_DBG("\r\n");
    PRINT_DBG("[%s]\tInit BSP\r\n", NOTE);

    akos_core_init();
    PRINT_DBG("[%s]\tInit AKOS v%u.%u.%u\r\n", NOTE, AKOS_VERSION_MAIN,
              AKOS_VERSION_SUB1, AKOS_VERSION_SUB2);
    PRINT_DBG("[%s]\tExample: THREAD\r\n", NOTE);
    PRINT_DBG("[%s]\tStart scheduler\r\n\r\n", NOTE);

    akos_core_run();

    while (true) {
    }
}
