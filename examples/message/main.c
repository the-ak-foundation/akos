#include "akos.h"
#include "board.h"

enum { MASTER_ID, SLAVE_ID };

static void master_thread(void* arg)
{
    (void)arg;

    for (;;) {
        akos_thread_post_msg_pure(SLAVE_ID, 1);
        akos_thread_delay(500u);
    }
}

static void slave_thread(void* arg)
{
    (void)arg;

    for (;;) {
        msg_t* message = akos_thread_wait_for_msg(OS_CFG_DELAY_MAX);

        if (message != NULL) {
            board_led_toggle();
            akos_message_free(message);
        }
    }
}

AKOS_THREAD_DEFINE(master, MASTER_ID, master_thread, NULL, 2u, 0u, 64u);
AKOS_THREAD_DEFINE(slave, SLAVE_ID, slave_thread, NULL, 1u, 4u, 64u);

int main(void)
{
    board_init();
    akos_core_init();
    akos_core_run();

    for (;;) {
    }
}
