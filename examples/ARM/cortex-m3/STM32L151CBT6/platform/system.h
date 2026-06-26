/**
  ******************************************************************************
  * @file    system.h
  * @brief   STM32L151CBT6 system clock API.
  ******************************************************************************
  */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t SystemCoreClock;

void SystemInit(void);
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_H__ */
