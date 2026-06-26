/**
  ******************************************************************************
  * @file    system.c
  * @brief   STM32F030F4P6 clock setup and SystemCoreClock helpers.
  *
  *   Clock tree used by this board example:
  *     HSI 8 MHz -> SYSCLK 8 MHz
  *     HCLK      -> SYSCLK / 1 -> 8 MHz
  *
  *   Cortex-M0 does not have VTOR, so the vector table stays at the start of
  *   FLASH as defined by the linker script.
  ******************************************************************************
  */

#include "stm32f0xx.h"

/* ============================================================
 * 1. Clock configuration
 * ============================================================ */

#ifndef HSI_VALUE
#define HSI_VALUE 8000000u
#endif

#ifndef HSE_VALUE
#define HSE_VALUE 8000000u
#endif

static const uint8_t ahb_presc_table[16] = {
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
    1u, 2u, 3u, 4u, 6u, 7u, 8u, 9u,
};

uint32_t SystemCoreClock = HSI_VALUE; // 8 MHz

/* ============================================================
 * 2. System API
 * ============================================================ */

void SystemInit(void)
{
    SystemCoreClock = HSI_VALUE;
}

void SystemCoreClockUpdate(void)
{
    uint32_t sysclk;
    uint32_t sws = RCC->CFGR & RCC_CFGR_SWS;

    switch (sws)
    {
    case RCC_CFGR_SWS_HSE:
        sysclk = HSE_VALUE;
        break;

    case RCC_CFGR_SWS_PLL:
    {
        uint32_t pllmul = ((RCC->CFGR & RCC_CFGR_PLLMUL) >> 18) + 2u;

        if ((RCC->CFGR & RCC_CFGR_PLLSRC) == RCC_CFGR_PLLSRC_HSE_PREDIV)
        {
            uint32_t prediv = (RCC->CFGR2 & RCC_CFGR2_PREDIV) + 1u;
            sysclk = (HSE_VALUE / prediv) * pllmul;
        }
        else
        {
            sysclk = (HSI_VALUE >> 1) * pllmul;
        }
        break;
    }

    case RCC_CFGR_SWS_HSI:
    default:
        sysclk = HSI_VALUE;
        break;
    }

    SystemCoreClock = sysclk >> ahb_presc_table[(RCC->CFGR & RCC_CFGR_HPRE) >> 4];
}
