#include <stdint.h>

#include "akos.h"
#include "board.h"
#include "console.h"

enum
{
    APP_THREAD_LED_ID = 0u,
    APP_THREAD_STRESS0_ID,
    APP_THREAD_STRESS1_ID,
    APP_THREAD_STRESS2_ID,
};

enum
{
    APP_SIG_LED_TOGGLE = 1,
};

static volatile uint32_t stress0_counter;
static volatile uint32_t stress1_counter;
static volatile uint32_t stress2_counter;

static uint32_t shell_parse_u32(const char *text, uint32_t fallback)
{
    uint32_t value = 0u;

    if (text == NULL)
    {
        return fallback;
    }

    while (*text != '\0')
    {
        if ((*text < '0') || (*text > '9'))
        {
            return fallback;
        }

        value = (value * 10u) + (uint32_t)(*text - '0');
        ++text;
    }

    return value;
}

static int shell_cmd_version(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    console_printf("AKOS shell + MCU load example\r\n");
    console_printf("Try: help, top, mcuload 500, counters, led toggle\r\n");
    return 0;
}

static int shell_cmd_led_on(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    board_led_on();
    console_printf("LED on\r\n");
    return 0;
}

static int shell_cmd_led_off(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    board_led_off();
    console_printf("LED off\r\n");
    return 0;
}

static int shell_cmd_led_toggle(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    board_led_toggle();
    console_printf("LED toggled\r\n");
    return 0;
}

static int shell_cmd_counters(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    console_printf("stress0=%lu stress1=%lu stress2=%lu\r\n",
                   (unsigned long)stress0_counter,
                   (unsigned long)stress1_counter,
                   (unsigned long)stress2_counter);
    return 0;
}

static int shell_cmd_mcuload(int argc, char **argv)
{
    uint32_t sample_ticks = 500u;
    uint32_t idle_before;
    uint32_t total_before;
    uint32_t idle_after;
    uint32_t total_after;
    uint32_t idle_delta;
    uint32_t total_delta;
    uint32_t busy_percent;
    uint32_t idle_percent;

    if (argc > 0)
    {
        sample_ticks = shell_parse_u32(argv[0], sample_ticks);
        if (sample_ticks == 0u)
        {
            sample_ticks = 1u;
        }
    }

    akos_thread_get_runtime_totals(&idle_before, &total_before);
    akos_thread_delay(sample_ticks);
    akos_thread_get_runtime_totals(&idle_after, &total_after);

    idle_delta = idle_after - idle_before;
    total_delta = total_after - total_before;

    if ((total_delta == 0u) || (idle_delta >= total_delta))
    {
        busy_percent = 0u;
        idle_percent = 100u;
    }
    else
    {
        busy_percent = ((total_delta - idle_delta) * 100u) / total_delta;
        idle_percent = 100u - busy_percent;
    }

    console_printf("sample=%lu ticks busy=%lu%% idle=%lu%%\r\n",
                   (unsigned long)sample_ticks,
                   (unsigned long)busy_percent,
                   (unsigned long)idle_percent);
    return 0;
}

AKOS_SHELL_CMD_DEFINE(shell_version_cmd, "version", "Show example info", shell_cmd_version);
AKOS_SHELL_CMD_DEFINE(shell_led_on_cmd, "led on", "Turn PB8 LED on", shell_cmd_led_on);
AKOS_SHELL_CMD_DEFINE(shell_led_off_cmd, "led off", "Turn PB8 LED off", shell_cmd_led_off);
AKOS_SHELL_CMD_DEFINE(shell_led_toggle_cmd, "led toggle", "Toggle PB8 LED", shell_cmd_led_toggle);
AKOS_SHELL_CMD_DEFINE(shell_counters_cmd, "counters", "Show worker counters", shell_cmd_counters);
AKOS_SHELL_CMD_DEFINE(shell_mcuload_cmd, "mcuload", "Sample busy/idle load in ticks", shell_cmd_mcuload);

static void led_thread(void *p_arg)
{
    ak_timer_t *blink_timer;

    (void)p_arg;

    blink_timer = akos_timer_create(0u,
                                    APP_SIG_LED_TOGGLE,
                                    NULL,
                                    APP_THREAD_LED_ID,
                                    250u,
                                    TIMER_PERIODIC);

    if (blink_timer != NULL)
    {
        akos_timer_start(blink_timer, 250u);
    }

    for (;;)
    {
        msg_t *msg = akos_thread_wait_for_msg(AKOS_CFG_DELAY_MAX);
        if ((msg != NULL) && (akos_message_get_pure_data(msg) == APP_SIG_LED_TOGGLE))
        {
            board_led_toggle();
        }

        if (msg != NULL)
        {
            akos_message_free(msg);
        }
    }
}

static void stress_thread0(void *p_arg)
{
    uint32_t index;

    (void)p_arg;

    for (;;)
    {
        for (index = 0u; index < 15000u; ++index)
        {
            ++stress0_counter;
            __asm volatile ("nop");
        }

        akos_thread_delay(1u);
    }
}

static void stress_thread1(void *p_arg)
{
    uint32_t index;

    (void)p_arg;

    for (;;)
    {
        for (index = 0u; index < 6000u; ++index)
        {
            stress1_counter += 3u;
            __asm volatile ("nop");
        }

        akos_thread_delay(2u);
    }
}

static void stress_thread2(void *p_arg)
{
    uint32_t index;

    (void)p_arg;

    for (;;)
    {
        for (index = 0u; index < 9000u; ++index)
        {
            stress2_counter += 5u;
            __asm volatile ("nop");
        }

        akos_thread_delay(3u);
    }
}

AKOS_THREAD_DEFINE(led_thread_desc, APP_THREAD_LED_ID, led_thread, NULL, 4u, 4u, 96u);
AKOS_THREAD_DEFINE(stress_thread0_desc, APP_THREAD_STRESS0_ID, stress_thread0, NULL, 2u, 0u, 96u);
AKOS_THREAD_DEFINE(stress_thread1_desc, APP_THREAD_STRESS1_ID, stress_thread1, NULL, 5u, 0u, 96u);
AKOS_THREAD_DEFINE(stress_thread2_desc, APP_THREAD_STRESS2_ID, stress_thread2, NULL, 7u, 0u, 96u);

int main(void)
{
    console_init();
    board_led_init();

    console_printf("\r\n[akos] shell/mcuload example start\r\n");

    akos_core_init();
    akos_core_run();

    for (;;)
    {
    }
}
