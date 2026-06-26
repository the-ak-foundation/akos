/**
  ******************************************************************************
  * @file    system.c
  * @brief   LP-MSPM0C1104 SystemCoreClock helpers.
  ******************************************************************************
  */

#include "system.h"

/* ============================================================
 * 1. Clock state
 * ============================================================ */

uint32_t SystemCoreClock = SYSTEM_CORE_CLOCK_HZ;

/* ============================================================
 * 2. System clock helpers
 * ============================================================ */

void SystemInit(void)
{
    SystemCoreClock = SYSTEM_CORE_CLOCK_HZ;
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = SYSTEM_CORE_CLOCK_HZ;
}
