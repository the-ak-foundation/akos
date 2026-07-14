#include "main.h"

#define NOTE "TASK_PRIO_HIGH"

void task_prio_high(void* p_arg) {
    ak_timer_t* timer =
        akos_timer_create(0u, MSG_SIGNAL_PRIO_HIGH, NULL, THREAD_PRIO_HIGH_ID,
                          HIGH_TASK_PERIOD_MS, TIMER_PERIODIC);
    if (timer != NULL) {
        akos_timer_start(timer, HIGH_TASK_PERIOD_MS);
    }

    while (true) {
        msg_t* message = akos_thread_wait_for_msg(OS_CFG_DELAY_MAX);

        if (message != NULL) {
            switch (message->sig) {
            case MSG_SIGNAL_PRIO_HIGH: {
                uint32_t ticks = akos_thread_get_tick();

                bsp_led_toggle();
                PRINT_DBG("[%02lu-%02lu-%02lu %03lu ms] [%s]\trecv signal, "
                          "toggle LED\r\n",
                          (unsigned long)TIME_HOURS(ticks),
                          (unsigned long)TIME_MINUTES(ticks),
                          (unsigned long)TIME_SECONDS(ticks),
                          (unsigned long)TIME_MILLIS(ticks), NOTE);
            } break;

            default: {
            } break;
            }

            akos_message_free(message);
        }
    }
}
