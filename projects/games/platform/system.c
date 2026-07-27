#include "system.h"

#include "game_hal.h"

static void system_console_putc(int character) {
    game_uart_putc((char)character);
}

void system_init_console(void) {
    xdev_out(system_console_putc);
}

void SysInitConsole(void) {
    system_init_console();
}
