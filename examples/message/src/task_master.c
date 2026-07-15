#include "main.h"

#define NOTE "TASK MASTER"

void task_master(void* p_arg) {
    (void)p_arg;

    ak_timer_t* timer = akos_timer_create(0u, MSG_SIGNAL_TIMER, NULL, THREAD_MASTER_ID,
                                          MESSAGE_PERIOD_MS, TIMER_PERIODIC);

    if (timer != NULL) {
        akos_timer_start(timer, MESSAGE_PERIOD_MS);
    }

    while (true) {
        msg_t* message = akos_thread_wait_for_msg(OS_CFG_DELAY_MAX);

        if (message != NULL) {
            switch (message->sig) {
            case MSG_SIGNAL_TIMER: {
#if EXAMPLE_DEBUG_ENABLED
                uint32_t ticks = akos_thread_get_tick();

                PRINT_DBG("[%02lu-%02lu-%02lu %03lu ms] [%s]\tsend \"%s\" to "
                          "task Slave\r\n",
                          (unsigned long)TIME_HOURS(ticks), (unsigned long)TIME_MINUTES(ticks),
                          (unsigned long)TIME_SECONDS(ticks), (unsigned long)TIME_MILLIS(ticks),
                          NOTE, MESSAGE_TEXT);
#endif

                akos_thread_post_msg_dynamic(THREAD_SLAVE_ID, MSG_SIGNAL_TEXT, MESSAGE_TEXT,
                                             (uint8_t)sizeof(MESSAGE_TEXT));

            } break;

            default: {
            } break;
            }

            akos_message_free(message);
        }
    }
}
