#include "main.h"

#define NOTE "TASK TIMER"

void task_timer_demo(void* p_arg) {
    (void)p_arg;

    ak_timer_t* timer = akos_timer_create(0u, MSG_SIGNAL_TIMER, NULL, THREAD_TIMER_DEMO_ID,
                                          TIMER_PERIOD_MS, TIMER_PERIODIC);

    if (timer != NULL) {
        akos_timer_start(timer, TIMER_PERIOD_MS);
    }

    while (true) {
        msg_t* message = akos_thread_wait_for_msg(OS_CFG_DELAY_MAX);

        if (message != NULL) {
            switch (message->sig) {
            case MSG_SIGNAL_TIMER: {
#if EXAMPLE_DEBUG_ENABLED
                uint32_t ticks = akos_thread_get_tick();
#endif

                bsp_led_toggle();
#if EXAMPLE_DEBUG_ENABLED
                PRINT_DBG("[%02lu-%02lu-%02lu %03lu ms] [%s]\ttimer expired, "
                          "toggle LED\r\n",
                          (unsigned long)TIME_HOURS(ticks), (unsigned long)TIME_MINUTES(ticks),
                          (unsigned long)TIME_SECONDS(ticks), (unsigned long)TIME_MILLIS(ticks),
                          NOTE);
#endif
            } break;

            default: {
            } break;
            }

            akos_message_free(message);
        }
    }
}
