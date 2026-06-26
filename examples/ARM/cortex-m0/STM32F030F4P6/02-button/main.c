/**
  ******************************************************************************
  * @file    main.c
  * @brief   STM32F030F4P6 AKOS button example.
  ******************************************************************************
  */

#include <stdbool.h>
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
 * 2. Button thread context
 * ============================================================ */

typedef struct
{
    uint32_t poll_ticks;
    uint32_t debounce_samples;
} button_thread_ctx_t;

static const button_thread_ctx_t button_ctx = {
    .poll_ticks = 10u,
    .debounce_samples = 3u,
};

/* ============================================================
 * 3. Thread entry
 * ============================================================ */

static void button_task(void *p_arg)
{
    const button_thread_ctx_t *ctx = (const button_thread_ctx_t *)p_arg;
    bool stable_pressed = board_button_is_pressed();
    bool last_sample = stable_pressed;
    uint32_t same_count = 0u;

    for (;;)
    {
        const bool sample = board_button_is_pressed();

        if (sample == last_sample)
        {
            if (same_count < ctx->debounce_samples)
            {
                ++same_count;
            }
        }
        else
        {
            last_sample = sample;
            same_count = 0u;
        }

        if ((same_count >= ctx->debounce_samples) && (sample != stable_pressed))
        {
            stable_pressed = sample;

            if (stable_pressed)
            {
                board_led_toggle();
            }
        }

        akos_thread_delay(ctx->poll_ticks);
    }
}

/* ============================================================
 * 4. Static thread registration
 * ============================================================ */

AKOS_THREAD_DEFINE(button_thread_desc, 0u, button_task, &button_ctx, 4u, 0u, 20u);

/* ============================================================
 * 5. Application entry
 * ============================================================ */

__attribute__((noreturn))
int main(void)
{
    board_init();

    akos_core_init();
    akos_core_run();

    for (;;)
    {
    }
}
