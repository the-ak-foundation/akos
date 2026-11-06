/**
 ******************************************************************************
 * @file    bsp.c
 * @brief   Game Hub board initialization and GPIO
 *
 * @module  Game Hub - Platform
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "bsp.h"

#include "io_cfg.h"
#include "uart.h"
#include "buzzer.h"

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
typedef struct {
    GPIO_TypeDef *port;
    uint8_t pin;
} bsp_gpio_t;

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/

static const bsp_gpio_t bsp_buttons[BSP_BUTTON_COUNT] = {
    [BSP_BUTTON_DOWN] = {BUTTON_DOWN_IO_PORT, BUTTON_DOWN_IO_PIN},
    [BSP_BUTTON_UP] = {BUTTON_UP_IO_PORT, BUTTON_UP_IO_PIN},
    [BSP_BUTTON_MODE] = {BUTTON_MODE_IO_PORT, BUTTON_MODE_IO_PIN},
};

/* Board LEDs are active high: life PB8, LED1 PA2, LED2 PA3. */
static const bsp_gpio_t bsp_leds[BSP_LED_COUNT] = {
    [BSP_LED_LIFE] = {LED_LIFE_IO_PORT, LED_LIFE_IO_PIN},
    [BSP_LED_1] = {LED_STATUS_1_IO_PORT, LED_STATUS_1_IO_PIN},
    [BSP_LED_2] = {LED_STATUS_2_IO_PORT, LED_STATUS_2_IO_PIN},
};

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void bsp_gpio_input_pull_up_init(GPIO_TypeDef *port, uint8_t pin) {
    const uint32_t shift = (uint32_t)pin * 2u;

    port->MODER &= ~(3u << shift);
    port->PUPDR = (port->PUPDR & ~(3u << shift)) | (1u << shift);
}

static void bsp_gpio_output_init(GPIO_TypeDef *port, uint8_t pin, bool high) {
    const uint32_t shift = (uint32_t)pin * 2u;
    const uint32_t mask = 1u << pin;

    port->MODER = (port->MODER & ~(3u << shift)) | (1u << shift);
    port->OTYPER &= ~mask;
    port->OSPEEDR &= ~(3u << shift);
    port->PUPDR &= ~(3u << shift);
    port->BSRR = high ? mask : (mask << 16u);
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void bsp_init(void) {
    RCC->AHBENR |= BUTTON_DOWN_IO_CLOCK | BUTTON_UP_IO_CLOCK | BUTTON_MODE_IO_CLOCK |
                   LED_LIFE_IO_CLOCK | LED_STATUS_1_IO_CLOCK | LED_STATUS_2_IO_CLOCK;
    for (uint32_t led = 0u; led < BSP_LED_COUNT; led++) {
        bsp_gpio_output_init(bsp_leds[led].port, bsp_leds[led].pin, false);
    }
    for (uint32_t button = 0u; button < BSP_BUTTON_COUNT; button++) {
        bsp_gpio_input_pull_up_init(bsp_buttons[button].port, bsp_buttons[button].pin);
    }
    uart_init();
    buzzer_init();
    buzzer_off();
}

bool bsp_button_is_pressed(bsp_button_t button) {
    if ((uint32_t)button >= BSP_BUTTON_COUNT) {
        return false;
    }

    return (bsp_buttons[button].port->IDR & (1u << bsp_buttons[button].pin)) == 0u;
}

void bsp_led_write(bsp_led_t led, bool on) {
    uint32_t mask;

    if ((uint32_t)led >= BSP_LED_COUNT) {
        return;
    }

    mask = 1u << bsp_leds[led].pin;
    bsp_leds[led].port->BSRR = on ? mask : (mask << 16u);
}

void bsp_led_toggle(bsp_led_t led) {
    bool is_on;

    if ((uint32_t)led >= BSP_LED_COUNT) {
        return;
    }

    is_on = (bsp_leds[led].port->ODR & (1u << bsp_leds[led].pin)) != 0u;
    bsp_led_write(led, !is_on);
}
