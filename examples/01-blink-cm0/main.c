#include <stdint.h>

#include "akos.h"
#include "board.h"

__attribute__((weak)) void _close(void)
{}
__attribute__((weak)) void _lseek(void)
{}
__attribute__((weak)) void _read(void)
{}
__attribute__((weak)) void _write(void)
{}

typedef struct
{
    uintptr_t gpio_base;
    uint32_t pin;
    uint32_t delay_ticks;
} blink_thread_ctx_t;

static const blink_thread_ctx_t blink_pa4_ctx = {
    .gpio_base = BLINK_GPIOA_BASE,
    .pin = BLINK_LED_PA4_PIN,
    .delay_ticks = 100u,
};

static void blink_task(void *p_arg)
{
    const blink_thread_ctx_t *ctx = (const blink_thread_ctx_t *)p_arg;

    for (;;)
    {
        board_gpio_toggle(ctx->gpio_base, ctx->pin);
        akos_thread_delay(ctx->delay_ticks);
    }
}

AKOS_THREAD_DEFINE(blink_pa4_thread_desc, 0u, blink_task, &blink_pa4_ctx, 4u, 0u, 20u);

__attribute__((noreturn))
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
