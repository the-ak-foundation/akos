#ifndef STM32F030F4P6_BLINK_BOARD_H
#define STM32F030F4P6_BLINK_BOARD_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f0xx.h"

#define BLINK_LED_GPIO GPIOA
#define BLINK_LED_PIN  4u

static inline void board_gpio_pin_output(GPIO_TypeDef *gpio, uint32_t pin)
{
    const uint32_t shift = pin * 2u;
    const uint32_t field_mask = 3u << shift;

    gpio->MODER = (gpio->MODER & ~field_mask) | (1u << shift);
    gpio->OTYPER &= ~(1u << pin);
    gpio->OSPEEDR &= ~field_mask;
    gpio->PUPDR &= ~field_mask;
    gpio->BSRR = 1u << (pin + 16u);
}

static inline void board_gpio_write(GPIO_TypeDef *gpio, uint32_t pin, bool on)
{
    if (on)
    {
        gpio->BSRR = 1u << pin;
    }
    else
    {
        gpio->BSRR = 1u << (pin + 16u);
    }
}

static inline void board_gpio_toggle(GPIO_TypeDef *gpio, uint32_t pin)
{
    board_gpio_write(gpio, pin, (gpio->ODR & (1u << pin)) == 0u);
}

static inline void board_leds_init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    (void)RCC->AHBENR;

    board_gpio_pin_output(BLINK_LED_GPIO, BLINK_LED_PIN);
}

static inline void board_leds_toggle(void)
{
    board_gpio_toggle(BLINK_LED_GPIO, BLINK_LED_PIN);
}

#endif /* STM32F030F4P6_BLINK_BOARD_H */
