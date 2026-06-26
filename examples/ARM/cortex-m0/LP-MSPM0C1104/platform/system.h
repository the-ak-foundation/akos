#ifndef LP_MSPM0C1104_SYSTEM_H
#define LP_MSPM0C1104_SYSTEM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYSTEM_CORE_CLOCK_HZ (24000000u)

extern uint32_t SystemCoreClock;

void SystemInit(void);
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* LP_MSPM0C1104_SYSTEM_H */
