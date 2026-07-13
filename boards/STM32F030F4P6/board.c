#include "board.h"

#include "stm32f0xx.h"
#include "system_stm32f0xx.h"

#define LED_PIN 4u

static void board_clock_init(void)
{
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0u)
    {
    }

    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE |
                   RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL);
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_DIV2 | RCC_CFGR_PLLMUL12;

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

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER = (GPIOA->MODER & ~(3u << (LED_PIN * 2u))) |
                   (1u << (LED_PIN * 2u));
    board_led_write(false);
}

void board_led_write(bool on)
{
    GPIOA->BSRR = on ? (1u << LED_PIN) : (1u << (LED_PIN + 16u));
}

void board_led_toggle(void)
{
    GPIOA->ODR ^= 1u << LED_PIN;
}
