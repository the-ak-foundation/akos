#ifndef AKOS_SAMPLE_00_BLINK_BOARD_H
#define AKOS_SAMPLE_00_BLINK_BOARD_H

/**
 * @file board.h
 * @brief Bare-metal GPIO helpers that use literal STM32L1 register addresses.
 */

#include <stdbool.h>
#include <stdint.h>

#ifndef BLINK_RCC_BASE
#define BLINK_RCC_BASE              0x40023800u
#endif

#ifndef BLINK_GPIOA_BASE
#define BLINK_GPIOA_BASE            0x40020000u
#endif

#ifndef BLINK_GPIOB_BASE
#define BLINK_GPIOB_BASE            0x40020400u
#endif

#define BLINK_RCC_AHBENR_OFFSET     0x1Cu

#define BLINK_GPIO_MODER_OFFSET     0x00u
#define BLINK_GPIO_OTYPER_OFFSET    0x04u
#define BLINK_GPIO_OSPEEDR_OFFSET   0x08u
#define BLINK_GPIO_PUPDR_OFFSET     0x0Cu
#define BLINK_GPIO_ODR_OFFSET       0x14u
#define BLINK_GPIO_BSRRL_OFFSET     0x18u
#define BLINK_GPIO_BSRRH_OFFSET     0x1Au

#define BLINK_RCC_AHBENR            (*(volatile uint32_t *)((uintptr_t)BLINK_RCC_BASE + BLINK_RCC_AHBENR_OFFSET))

#define BLINK_GPIO_MODER(base)      (*(volatile uint32_t *)((uintptr_t)(base) + BLINK_GPIO_MODER_OFFSET))
#define BLINK_GPIO_OTYPER(base)     (*(volatile uint16_t *)((uintptr_t)(base) + BLINK_GPIO_OTYPER_OFFSET))
#define BLINK_GPIO_OSPEEDR(base)    (*(volatile uint32_t *)((uintptr_t)(base) + BLINK_GPIO_OSPEEDR_OFFSET))
#define BLINK_GPIO_PUPDR(base)      (*(volatile uint32_t *)((uintptr_t)(base) + BLINK_GPIO_PUPDR_OFFSET))
#define BLINK_GPIO_ODR(base)        (*(volatile uint16_t *)((uintptr_t)(base) + BLINK_GPIO_ODR_OFFSET))
#define BLINK_GPIO_BSRRL(base)      (*(volatile uint16_t *)((uintptr_t)(base) + BLINK_GPIO_BSRRL_OFFSET))
#define BLINK_GPIO_BSRRH(base)      (*(volatile uint16_t *)((uintptr_t)(base) + BLINK_GPIO_BSRRH_OFFSET))

#define BLINK_LED_PB8_PIN           8u
#define BLINK_LED_PA2_PIN           2u
#define BLINK_LED_PA3_PIN           3u

static inline void board_led_clock_enable(void)
{
    BLINK_RCC_AHBENR |= (1u << 0) | (1u << 1);
}

static inline void board_gpio_pin_output(uintptr_t gpio_base, uint32_t pin)
{
    const uint32_t shift = pin * 2u;
    const uint32_t field_mask = (3u << shift);

    BLINK_GPIO_MODER(gpio_base) = (BLINK_GPIO_MODER(gpio_base) & ~field_mask) | (1u << shift);
    BLINK_GPIO_OTYPER(gpio_base) &= (uint16_t)~(1u << pin);
    BLINK_GPIO_OSPEEDR(gpio_base) &= ~field_mask;
    BLINK_GPIO_PUPDR(gpio_base) &= ~field_mask;
    BLINK_GPIO_BSRRH(gpio_base) = (uint16_t)(1u << pin);
}

static inline void board_gpio_write(uintptr_t gpio_base, uint32_t pin, bool on)
{
    if (on)
    {
        BLINK_GPIO_BSRRL(gpio_base) = (uint16_t)(1u << pin);
    }
    else
    {
        BLINK_GPIO_BSRRH(gpio_base) = (uint16_t)(1u << pin);
    }
}

static inline void board_gpio_toggle(uintptr_t gpio_base, uint32_t pin)
{
    if ((BLINK_GPIO_ODR(gpio_base) & (1u << pin)) != 0u)
    {
        board_gpio_write(gpio_base, pin, false);
    }
    else
    {
        board_gpio_write(gpio_base, pin, true);
    }
}

static inline void board_leds_init(void)
{
    board_led_clock_enable();
    board_gpio_pin_output(BLINK_GPIOB_BASE, BLINK_LED_PB8_PIN);
    board_gpio_pin_output(BLINK_GPIOA_BASE, BLINK_LED_PA2_PIN);
    board_gpio_pin_output(BLINK_GPIOA_BASE, BLINK_LED_PA3_PIN);
}

static inline void board_led_pb8_toggle(void)
{
    board_gpio_toggle(BLINK_GPIOB_BASE, BLINK_LED_PB8_PIN);
}

static inline void board_leds_toggle(void)
{
    board_gpio_toggle(BLINK_GPIOB_BASE, BLINK_LED_PB8_PIN);
    board_gpio_toggle(BLINK_GPIOA_BASE, BLINK_LED_PA2_PIN);
    board_gpio_toggle(BLINK_GPIOA_BASE, BLINK_LED_PA3_PIN);
}

#endif /* AKOS_SAMPLE_00_BLINK_BOARD_H */
