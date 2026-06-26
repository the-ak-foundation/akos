/**
  ******************************************************************************
  * @file    system.c
  * @brief   STM32L151CBT6 clock setup and SystemCoreClock helpers.
  *
  *   Clock tree used by this board example:
  *     HSE 8 MHz -> PLL MUL 12 / DIV 3 -> SYSCLK 32 MHz
  *     HCLK      -> SYSCLK / 1          -> 32 MHz
  *     PCLK1     -> HCLK / 1            -> 32 MHz
  *     PCLK2     -> HCLK / 1            -> 32 MHz
  *
  *   The image is linked at 0x08003000, so SystemInit() also relocates VTOR
  *   to that application offset.
  ******************************************************************************
  */

#include "stm32l1xx.h"

#ifndef HSI_VALUE
#define HSI_VALUE 16000000u
#endif

#ifndef HSE_VALUE
#define HSE_VALUE 8000000u
#endif

#ifndef HSE_STARTUP_TIMEOUT
#define HSE_STARTUP_TIMEOUT 100u
#endif

/* ============================================================
 * 1. Clock configuration
 * ============================================================ */

#define APP_VECTOR_OFFSET 0x3000u

static const uint8_t pll_mul_table[9] = {
    3u, 4u, 6u, 8u, 12u, 16u, 24u, 32u, 48u,
};

static const uint8_t ahb_presc_table[16] = {
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
    1u, 2u, 3u, 4u, 6u, 7u, 8u, 9u,
};

uint32_t SystemCoreClock = 32000000u; // 32 MHz

/* ============================================================
 * 2. Private helpers
 * ============================================================ */

static void system_clock_config(void);

static uint32_t msi_clock_hz(void)
{
    const uint32_t range = (RCC->ICSCR & RCC_ICSCR_MSIRANGE) >> 13;

    return 32768u * (1u << (range + 1u));
}

/* ============================================================
 * 3. System API
 * ============================================================ */

void SystemInit(void)
{
    RCC->CR |= RCC_CR_MSION;

    RCC->CFGR &= 0x88FFC00Cu;
    RCC->CR &= 0xEEFEFFFEu;
    RCC->CR &= 0xFFFBFFFFu;
    RCC->CFGR &= 0xFF02FFFFu;
    RCC->CIR = 0u;

    system_clock_config();

    SCB->VTOR = FLASH_BASE | APP_VECTOR_OFFSET;
}

void SystemCoreClockUpdate(void)
{
    uint32_t sysclk;
    uint32_t sws = RCC->CFGR & RCC_CFGR_SWS;

    switch (sws)
    {
    case RCC_CFGR_SWS_HSI:
        sysclk = HSI_VALUE;
        break;

    case RCC_CFGR_SWS_HSE:
        sysclk = HSE_VALUE;
        break;

    case RCC_CFGR_SWS_PLL:
    {
        const uint32_t pllmul = pll_mul_table[(RCC->CFGR & RCC_CFGR_PLLMUL) >> 18];
        const uint32_t plldiv = ((RCC->CFGR & RCC_CFGR_PLLDIV) >> 22) + 1u;
        const uint32_t pllsrc = RCC->CFGR & RCC_CFGR_PLLSRC;
        const uint32_t pll_input = (pllsrc == RCC_CFGR_PLLSRC_HSE) ? HSE_VALUE : HSI_VALUE;

        sysclk = (pll_input * pllmul) / plldiv;
        break;
    }

    case RCC_CFGR_SWS_MSI:
    default:
        sysclk = msi_clock_hz();
        break;
    }

    SystemCoreClock = sysclk >> ahb_presc_table[(RCC->CFGR & RCC_CFGR_HPRE) >> 4];
}

/* ============================================================
 * 4. Board clock setup
 * ============================================================ */

static void system_clock_config(void)
{
    uint32_t startup_counter = 0u;

    RCC->CR |= RCC_CR_HSEON;

    while (((RCC->CR & RCC_CR_HSERDY) == 0u) &&
           (startup_counter < HSE_STARTUP_TIMEOUT))
    {
        ++startup_counter;
    }

    if ((RCC->CR & RCC_CR_HSERDY) == 0u)
    {
        SystemCoreClockUpdate();
        return;
    }

    FLASH->ACR |= FLASH_ACR_ACC64 | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY;

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR = PWR_CR_VOS_0;

    while ((PWR->CSR & PWR_CSR_VOSF) != 0u)
    {
    }

    RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1;

    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL12 | RCC_CFGR_PLLDIV3;

    RCC->CR |= RCC_CR_PLLON;

    while ((RCC->CR & RCC_CR_PLLRDY) == 0u)
    {
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
    {
    }

    SystemCoreClockUpdate();
}
