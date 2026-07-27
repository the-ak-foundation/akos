#include <stdbool.h>

#include "app.h"
#include "game_hal.h"
#include "system.h"
#include "task_list.h"

int main(void) {
    game_hal_init();
    game_led_write(GAME_LED_1, true);
    system_init_console();
    SYS_PRINT("Game Hub starting\n");

    akos_core_init();
    akos_core_run();

    while (true) {
    }
}
