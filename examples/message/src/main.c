#include "main.h"

#if EXAMPLE_DEBUG_ENABLED
#include <stdarg.h>
#include <stdio.h>
#endif

#define NOTE "MAIN"

AKOS_THREAD_DEFINE(master, THREAD_MASTER_ID, task_master, NULL, 1u, 4u,
                   APP_TASK_STACK_SIZE);
AKOS_THREAD_DEFINE(slave, THREAD_SLAVE_ID, task_slave, NULL, 2u, 4u,
                   APP_TASK_STACK_SIZE);

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
    PRINT_DBG("[%s]\tExample: MESSAGE\r\n", NOTE);
    PRINT_DBG("[%s]\tStart scheduler\r\n\r\n", NOTE);

    akos_core_run();

    while (true) {
    }
}

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
