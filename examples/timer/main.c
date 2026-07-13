#include "akos.h"
#include "board.h"

static void timer_thread(void *arg)
{
    ak_timer_t *timer;

    (void)arg;
    timer = akos_timer_create(0u, 1, NULL, 0u, 500u, TIMER_PERIODIC);
    if (timer != NULL)
    {
        akos_timer_start(timer, 500u);
    }

    for (;;)
    {
        msg_t *message = akos_thread_wait_for_msg(OS_CFG_DELAY_MAX);

        if (message != NULL)
        {
            board_led_toggle();
            akos_message_free(message);
        }
    }
}

AKOS_THREAD_DEFINE(timer_demo, 0u, timer_thread, NULL, 2u, 4u, 64u);

int main(void)
{
    board_init();
    akos_core_init();
    akos_core_run();

    for (;;)
    {
    }
}
