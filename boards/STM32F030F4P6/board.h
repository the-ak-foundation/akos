#ifndef AKOS_BOARD_H
#define AKOS_BOARD_H

#include <stdbool.h>

void board_init(void);
void board_led_write(bool on);
void board_led_toggle(void);

#endif
