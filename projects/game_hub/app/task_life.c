#include "app.h"

#include "game_hal.h"

void task_life(void *argument) {
    (void)argument;

    while (true) {
        game_led_toggle(GAME_LED_LIFE);
        akos_thread_delay(1000u);
    }
}
