#include "board.h"

#include "stm32l1xx.h"
#include "system_stm32l1xx.h"

#define LED_PIN 6u

static void board_clock_init(void)
{
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0u)
    {
    }

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR = (PWR->CR & ~PWR_CR_VOS) | PWR_CR_VOS_0;
    while ((PWR->CSR & PWR_CSR_VOSF) != 0u)
    {
    }

    FLASH->ACR = FLASH_ACR_ACC64 | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY;

    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE1 |
                   RCC_CFGR_PPRE2 | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL |
                   RCC_CFGR_PLLDIV);
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL6 |
                 RCC_CFGR_PLLDIV3;

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

void board_init(void)
{
    board_clock_init();

    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER = (GPIOB->MODER & ~(3u << (LED_PIN * 2u))) |
                   (1u << (LED_PIN * 2u));
    board_led_write(false);
}

void board_led_write(bool on)
{
    GPIOB->BSRR = on ? (1u << LED_PIN) : (1u << (LED_PIN + 16u));
}

void board_led_toggle(void)
{
    GPIOB->ODR ^= 1u << LED_PIN;
}
