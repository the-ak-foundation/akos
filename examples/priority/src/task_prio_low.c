#include "main.h"

#define NOTE "TASK_PRIO_LOW"

void task_prio_low(void* p_arg) {
    ak_timer_t* timer;

    timer = akos_timer_create(1u, MSG_SIGNAL_PRIO_LOW, NULL, THREAD_PRIO_LOW_ID,
                              LOW_TASK_PERIOD_MS, TIMER_PERIODIC);
    if (timer != NULL) {
        akos_timer_start(timer, LOW_TASK_PERIOD_MS);
    }

    while (true) {
        msg_t* message = akos_thread_wait_for_msg(OS_CFG_DELAY_MAX);

        if (message != NULL) {
            switch (message->sig) {
            case MSG_SIGNAL_PRIO_LOW: {
                uint32_t ticks = akos_thread_get_tick();

                PRINT_DBG("[%02lu-%02lu-%02lu %03lu ms] [%s]\trecv signal, "
                          "running\r\n",
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
