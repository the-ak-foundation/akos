/**
 * @file main.c
 * @brief AKOS blink sample with three independent LED threads.
 */

#include <stdint.h>

#include "akos.h"
#include "board.h"

typedef struct
{
    uintptr_t gpio_base;
    uint32_t pin;
    uint32_t delay_ticks;
} blink_thread_ctx_t;

static void blink_task(void *p_arg)
{
    const blink_thread_ctx_t *ctx = (const blink_thread_ctx_t *)p_arg;

    for (;;)
    {
        board_gpio_toggle(ctx->gpio_base, ctx->pin);
        akos_thread_delay(ctx->delay_ticks);
    }
}

static const blink_thread_ctx_t blink_pb8_ctx = {
    .gpio_base = BLINK_GPIOB_BASE,
    .pin = BLINK_LED_PB8_PIN,
    .delay_ticks = 250u,
};

static const blink_thread_ctx_t blink_pa2_ctx = {
    .gpio_base = BLINK_GPIOA_BASE,
    .pin = BLINK_LED_PA2_PIN,
    .delay_ticks = 400u,
};

static const blink_thread_ctx_t blink_pa3_ctx = {
    .gpio_base = BLINK_GPIOA_BASE,
    .pin = BLINK_LED_PA3_PIN,
    .delay_ticks = 650u,
};

AKOS_THREAD_DEFINE(blink_pb8_thread_desc, 0u, blink_task, &blink_pb8_ctx, 4u, 0u, 20u);
AKOS_THREAD_DEFINE(blink_pa2_thread_desc, 1u, blink_task, &blink_pa2_ctx, 5u, 0u, 20u);
AKOS_THREAD_DEFINE(blink_pa3_thread_desc, 2u, blink_task, &blink_pa3_ctx, 6u, 0u, 20u);

int main(void)
{
    board_leds_init();

    akos_core_init();
    akos_core_run();

    for (;;)
    {
        /* akos_core_run() should not return. */
    }
}
