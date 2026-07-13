#include "akos.h"
#include "board.h"

static void high_priority_thread(void *arg)
{
    (void)arg;

    for (;;)
    {
        board_led_toggle();
        akos_thread_delay(100u);
    }
}

static void low_priority_thread(void *arg)
{
    (void)arg;

    for (;;)
    {
        akos_thread_delay(1000u);
    }
}

AKOS_THREAD_DEFINE(high_priority, 0u, high_priority_thread, NULL, 1u, 0u, 64u);
AKOS_THREAD_DEFINE(low_priority, 1u, low_priority_thread, NULL, 2u, 0u, 64u);

int main(void)
{
    board_init();
    akos_core_init();
    akos_core_run();

    for (;;)
    {
    }
}
