#include "app.h"

#include <stdbool.h>

#include "game_hal.h"
#include "system.h"
#include "task_list.h"

#define NOTE "GAME_HUB"

int main(void) {
    game_hal_init();
    game_led_write(GAME_LED_1, true);
    system_init_console();

    SYS_PRINT("Game Hub starting\r\n");
    SYS_PRINT("[%s]\tInit BSP\r\n", NOTE);
    akos_core_init();
    SYS_PRINT("[%s]\tInit AKOS v%u.%u.%u\r\n", NOTE, AKOS_VERSION_MAIN,
              AKOS_VERSION_SUB1, AKOS_VERSION_SUB2);
    SYS_PRINT("[%s]\tProject: Game Hub\r\n", NOTE);
    SYS_PRINT("[%s]\tStart scheduler\r\n\r\n", NOTE);
    akos_core_run();

    while (true) {
    }
}
