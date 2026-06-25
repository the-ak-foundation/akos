#ifndef EXAMPLES_01_SHELL_MCULOAD_SYSTEM_H
#define EXAMPLES_01_SHELL_MCULOAD_SYSTEM_H

#include "console.h"

#ifndef SYS_PRINT
#define SYS_PRINT(...) console_printf(__VA_ARGS__)
#endif

#endif /* EXAMPLES_01_SHELL_MCULOAD_SYSTEM_H */
