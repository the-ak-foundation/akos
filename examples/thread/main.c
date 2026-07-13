#include "akos.h"
#include "board.h"

static void task_a_thread(void *arg)
{
    (void)arg;

    for (;;)
    {
        board_led_write(true);
        akos_thread_delay(500u);
    }
}

static void task_b_thread(void *arg)
{
    (void)arg;

    for (;;)
    {
        board_led_write(false);
        akos_thread_delay(1000u);
    }
}

AKOS_THREAD_DEFINE(task_a, 0u, task_a_thread, NULL, 1u, 0u, 64u);
AKOS_THREAD_DEFINE(task_b, 1u, task_b_thread, NULL, 2u, 0u, 64u);

int main(void)
{
    board_init();
    akos_core_init();
    akos_core_run();

    for (;;)
    {
    }
}
