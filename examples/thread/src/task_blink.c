#include "main.h"

#define NOTE "TASK BLINK"

void blink_task(void* p_arg) {
    const blink_task_ctx_t* context = (const blink_task_ctx_t*)p_arg;

    while (true) {
        bsp_led_toggle();
#if THREAD_TASK_LOG_ENABLED
        PRINT_DBG("[%s]\t%s toggle LED\r\n", NOTE, context->name);
#endif
        akos_thread_delay(context->delay_ticks);
    }
}
