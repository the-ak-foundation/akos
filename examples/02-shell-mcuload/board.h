#ifndef EXAMPLES_01_SHELL_MCULOAD_BOARD_H
#define EXAMPLES_01_SHELL_MCULOAD_BOARD_H

#include <stdint.h>

#include "stm32l1xx.h"

#define BOARD_LED_GPIO                GPIOB
#define BOARD_LED_PIN                 8u
#define BOARD_LED_MASK                (1u << BOARD_LED_PIN)
#define BOARD_LED_GPIO_EN_BIT         RCC_AHBENR_GPIOBEN

static inline void board_led_init(void)
{
    RCC->AHBENR |= BOARD_LED_GPIO_EN_BIT;

    BOARD_LED_GPIO->MODER &= ~(0x3u << (BOARD_LED_PIN * 2u));
    BOARD_LED_GPIO->MODER |= (0x1u << (BOARD_LED_PIN * 2u));
    BOARD_LED_GPIO->OTYPER &= ~BOARD_LED_MASK;
    BOARD_LED_GPIO->OSPEEDR &= ~(0x3u << (BOARD_LED_PIN * 2u));
    BOARD_LED_GPIO->OSPEEDR |= (0x1u << (BOARD_LED_PIN * 2u));
    BOARD_LED_GPIO->PUPDR &= ~(0x3u << (BOARD_LED_PIN * 2u));
    BOARD_LED_GPIO->BSRRH = (uint16_t)BOARD_LED_MASK;
}

static inline void board_led_on(void)
{
    BOARD_LED_GPIO->BSRRL = (uint16_t)BOARD_LED_MASK;
}

static inline void board_led_off(void)
{
    BOARD_LED_GPIO->BSRRH = (uint16_t)BOARD_LED_MASK;
}

static inline void board_led_toggle(void)
{
    if ((BOARD_LED_GPIO->ODR & BOARD_LED_MASK) != 0u)
    {
        board_led_off();
    }
    else
    {
        board_led_on();
    }
}

#endif /* EXAMPLES_01_SHELL_MCULOAD_BOARD_H */
