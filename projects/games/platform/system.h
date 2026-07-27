#ifndef __GAME_SYSTEM_H__
#define __GAME_SYSTEM_H__

#include "xprintf.h"

#define SYS_PRINT(format, ...) xprintf((format), ##__VA_ARGS__)

void system_init_console(void);
void SysInitConsole(void);

#endif /* __GAME_SYSTEM_H__ */
