#ifndef EXAMPLES_01_SHELL_MCULOAD_CONSOLE_H
#define EXAMPLES_01_SHELL_MCULOAD_CONSOLE_H

#include "xprintf.h"

#define console_printf xprintf

void console_init(void);
void console_write_char(int ch);

#endif /* EXAMPLES_01_SHELL_MCULOAD_CONSOLE_H */
