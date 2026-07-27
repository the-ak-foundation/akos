#include "task_shell.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "boot_shared.h"
#include "game_hal.h"
#include "xprintf.h"

#define SHELL_LINE_SIZE (64u)

typedef void (*shell_command_fn_t)(const char *arguments);

typedef struct {
    const char *name;
    shell_command_fn_t function;
    const char *help;
} shell_command_t;

static void shell_help(const char *arguments);
static void shell_lcd(const char *arguments);
static void shell_buzzer(const char *arguments);
static void shell_reboot(const char *arguments);
static void shell_fwu(const char *arguments);

static const shell_command_t shell_commands[] = {
    { "help",       shell_help,         "show commands" },
    { "lcd",        shell_lcd,          "lcd init|on|off|clear|white|checker|pixel|dump|game" },
    { "beep",       shell_buzzer,       "beep <frequency_hz> [duration_ms] | off" },
    { "reboot",     shell_reboot,       "reset the MCU" },
    { "fwu",        shell_fwu,          "fwu [info]: request UART firmware update" },
};

static const char *shell_skip_spaces(const char *text) {
    while ((text != NULL) && (*text == ' ')) {
        text++;
    }
    return text;
}

static void shell_prompt(void) {
    xputs("game_hub> ");
}

static void shell_help(const char *arguments) {
    (void)arguments;

    xputs("Commands:\n");
    for (size_t index = 0u;
         index < sizeof(shell_commands) / sizeof(shell_commands[0]); index++) {
        xprintf("  %-8s %s\n", shell_commands[index].name,
                shell_commands[index].help);
    }
}

static void shell_lcd_dump(void) {
    const uint8_t *buffer;
    size_t size = 0u;

    buffer = game_display_get_buffer(&size);
    xprintf("LCD framebuffer: %u bytes\n", (unsigned int)size);
    for (size_t index = 0u; index < size; index++) {
        if ((index & 15u) == 0u) {
            xprintf("%04X: ", (unsigned int)index);
        }
        xprintf("%02X%c", buffer[index],
                ((index & 15u) == 15u) ? '\n' : ' ');
    }
}

static void shell_lcd(const char *arguments) {
    arguments = shell_skip_spaces(arguments);

    if ((arguments == NULL) || (*arguments == '\0')) {
        xputs("Usage: lcd init|on|off|clear|white|checker|pixel|dump|game\n");
        return;
    }

    if (strcmp(arguments, "game") == 0) {
        game_app_display_resume();
        xputs("Game rendering resumed\n");
        return;
    }

    game_app_display_pause();
    akos_thread_delay(20u);

    if (strcmp(arguments, "init") == 0) {
        game_display_init();
        xputs("LCD initialized\n");
    } else if (strcmp(arguments, "on") == 0) {
        game_display_set_power(true);
    } else if (strcmp(arguments, "off") == 0) {
        game_display_set_power(false);
    } else if (strcmp(arguments, "clear") == 0) {
        game_display_clear();
        game_display_present();
    } else if (strcmp(arguments, "white") == 0) {
        game_display_fill_rect(0u, 0u, GAME_DISPLAY_WIDTH,
                               GAME_DISPLAY_HEIGHT, true);
        game_display_present();
    } else if (strcmp(arguments, "checker") == 0) {
        game_display_clear();
        for (uint8_t y = 0u; y < GAME_DISPLAY_HEIGHT; y += 8u) {
            for (uint8_t x = 0u; x < GAME_DISPLAY_WIDTH; x += 8u) {
                if ((((uint8_t)(x / 8u) + (uint8_t)(y / 8u)) & 1u) != 0u) {
                    game_display_fill_rect(x, y, 8u, 8u, true);
                }
            }
        }
        game_display_present();
    } else if (strcmp(arguments, "pixel") == 0) {
        game_display_clear();
        game_display_draw_pixel(9u, 1u, true);
        game_display_draw_pixel(13u, 4u, true);
        game_display_draw_pixel(16u, 7u, true);
        game_display_present();
    } else if (strcmp(arguments, "dump") == 0) {
        shell_lcd_dump();
    } else {
        xprintf("Unknown lcd option: %s\n", arguments);
    }
}

static void shell_buzzer(const char *arguments) {
    char *end;
    unsigned long frequency;
    unsigned long duration = 200u;

    arguments = shell_skip_spaces(arguments);
    if ((arguments == NULL) || (*arguments == '\0')) {
        xputs("Usage: beep <frequency_hz> [duration_ms] | off\n");
        return;
    }
    if (strcmp(arguments, "off") == 0) {
        game_buzzer_off();
        return;
    }

    frequency = strtoul(arguments, &end, 10);
    end = (char *)shell_skip_spaces(end);
    if (*end != '\0') {
        duration = strtoul(end, &end, 10);
    }
    if ((frequency < 20u) || (frequency > 20000u) ||
        (duration > 5000u)) {
        xputs("Range: frequency 20..20000 Hz, duration 0..5000 ms\n");
        return;
    }

    game_buzzer_init();
    game_buzzer_tone((uint16_t)frequency);
    if (duration > 0u) {
        akos_thread_delay((uint32_t)duration);
        game_buzzer_off();
    }
}

static void shell_reboot(const char *arguments) {
    (void)arguments;
    xputs("Rebooting...\n");
    akos_thread_delay(20u);
    game_system_reboot();
}

static void shell_fwu(const char *arguments) {
    boot_shared_app_command_t command;

    arguments = shell_skip_spaces(arguments);
    if ((arguments != NULL) && (strcmp(arguments, "info") == 0)) {
        boot_shared_get_app_command(&command);
        xprintf("BSF app cmd=%u container=%u io=%u dst=0x%08lX src=0x%08lX\n",
                command.command, command.container, command.io_driver,
                (unsigned long)command.destination_address,
                (unsigned long)command.source_address);
        return;
    }
    if ((arguments != NULL) && (*arguments != '\0')) {
        xputs("Usage: fwu [info]\n");
        return;
    }

    if (boot_shared_request_uart_update()) {
        /* Match the reference shell: reset immediately after arming BSF. */
        game_system_reboot();
    } else {
        xputs("ERROR: cannot write boot share; reset cancelled.\n");
    }
}

static void shell_execute(char *line) {
    char *arguments = line;

    while ((*arguments != '\0') && (*arguments != ' ')) {
        arguments++;
    }
    if (*arguments != '\0') {
        *arguments++ = '\0';
    }

    for (size_t index = 0u;
         index < sizeof(shell_commands) / sizeof(shell_commands[0]); index++) {
        if (strcmp(line, shell_commands[index].name) == 0) {
            shell_commands[index].function(arguments);
            return;
        }
    }

    xprintf("Unknown command: %s. Type 'help'.\n", line);
}

void task_shell(void *argument) {
    char line[SHELL_LINE_SIZE];
    size_t length = 0u;

    (void)argument;
    xputs("\nAKOS Game Hub shell\n");
    shell_prompt();

    while (true) {
        char character;

        if (!game_uart_try_getc(&character)) {
            akos_thread_delay(1u);
            continue;
        }

        if ((character == '\r') || (character == '\n')) {
            xputc('\n');
            if (length > 0u) {
                line[length] = '\0';
                shell_execute(line);
                length = 0u;
            }
            shell_prompt();
        } else if ((character == '\b') || (character == 0x7Fu)) {
            if (length > 0u) {
                length--;
                xputs("\b \b");
            }
        } else if ((character >= ' ') && (length + 1u < sizeof(line))) {
            line[length++] = character;
            game_uart_putc(character);
        }
    }
}
