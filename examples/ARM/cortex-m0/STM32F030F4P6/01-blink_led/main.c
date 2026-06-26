/**
  ******************************************************************************
  * @file    main.c
  * @brief   STM32F030F4P6 AKOS blink LED example.
  ******************************************************************************
  */

#include <stdint.h>

#include "akos.h"
#include "board.h"

/* ============================================================
 * 1. Newlib syscall stubs
 * ============================================================ */

__attribute__((weak)) void _close(void)
{
}

__attribute__((weak)) void _lseek(void)
{
}

__attribute__((weak)) void _read(void)
{
}

__attribute__((weak)) void _write(void)
{
}

/* ============================================================
 * 2. Blink thread context
 * ============================================================ */

typedef struct
{
    GPIO_TypeDef *gpio;
    uint32_t pin;
    uint32_t delay_ticks;
} blink_thread_ctx_t;

static const blink_thread_ctx_t blink_led_ctx = {
    .gpio = BLINK_LED_GPIO,
    .pin = BLINK_LED_PIN,
    .delay_ticks = 100u,
};

/* ============================================================
 * 3. Thread entry
 * ============================================================ */

static void blink_task(void *p_arg)
{
    const blink_thread_ctx_t *ctx = (const blink_thread_ctx_t *)p_arg;

    for (;;)
    {
        board_gpio_toggle(ctx->gpio, ctx->pin);
        akos_thread_delay(ctx->delay_ticks);
    }
}

/* ============================================================
 * 4. Static thread registration
 * ============================================================ */

AKOS_THREAD_DEFINE(blink_led_thread_desc, 0u, blink_task, &blink_led_ctx, 4u, 0u, 20u);

/* ============================================================
 * 5. Application entry
 * ============================================================ */

__attribute__((noreturn))
int main(void)
{
    board_leds_init();

    akos_core_init();
    akos_core_run();

    for (;;)
    {
    }
}
