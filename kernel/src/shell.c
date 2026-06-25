#include "shell.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "message.h"
#include "system.h"
#include "thread.h"

#if (OS_CFG_USE_SHELL != 0u)

#define AKOS_SHELL_SIG_RX_READY 1
#define AKOS_SHELL_TOP_REFRESH_TICKS 1000u
#define AKOS_SHELL_TOP_MAX_THREADS UINT8_MAX

extern const akos_shell_cmd_t __start_shell_cmd_desc[] __attribute__((weak));
extern const akos_shell_cmd_t __stop_shell_cmd_desc[] __attribute__((weak));

static volatile uint8_t shell_rx_head;
static volatile uint8_t shell_rx_tail;
static uint8_t shell_rx_buffer[OS_CFG_SHELL_RX_BUFFER_SIZE];
static volatile bool shell_rx_notified;

static char shell_line_buffer[OS_CFG_SHELL_LINE_BUFFER_SIZE];
static uint8_t shell_line_length;
static bool shell_top_active;
static bool shell_top_has_snapshot;
static uint32_t shell_top_prev_idle_ticks;
static uint32_t shell_top_prev_total_ticks;
static uint32_t shell_top_prev_thread_ticks[AKOS_SHELL_TOP_MAX_THREADS];
static uint8_t shell_top_thread_prio[AKOS_SHELL_TOP_MAX_THREADS];
static uint32_t shell_top_thread_load[AKOS_SHELL_TOP_MAX_THREADS];
static const char *shell_top_thread_name[AKOS_SHELL_TOP_MAX_THREADS];

static void shell_prompt(void);
static void shell_process_rx(void);
static void shell_process_byte(uint8_t byte);
static void shell_execute_line(void);
static int shell_tokenize(char *line, char **argv, int argv_max);
static void shell_show_help(void);
static void shell_show_top(void);
static void shell_top_enter(void);
static void shell_top_exit(void);
static bool shell_match_path(const char *path, int argc, char **argv, int *consumed_argc);

void akos_shell_feature_init(void)
{
    shell_rx_head = 0u;
    shell_rx_tail = 0u;
    shell_line_length = 0u;
    shell_rx_notified = false;
    shell_top_active = false;
    shell_top_has_snapshot = false;
    shell_top_prev_idle_ticks = 0u;
    shell_top_prev_total_ticks = 0u;
    memset(shell_top_prev_thread_ticks, 0x00, sizeof(shell_top_prev_thread_ticks));
    memset(shell_top_thread_prio, 0x00, sizeof(shell_top_thread_prio));
    memset(shell_top_thread_load, 0x00, sizeof(shell_top_thread_load));
    memset(shell_top_thread_name, 0x00, sizeof(shell_top_thread_name));
}

void akos_shell_thread(void *p_arg)
{
    (void)p_arg;

    SYS_PRINT("\r\n[akos-shell] ready\r\n");
    shell_prompt();

    for (;;)
    {
        msg_t *msg = akos_thread_wait_for_msg(shell_top_active ? AKOS_SHELL_TOP_REFRESH_TICKS : OS_CFG_DELAY_MAX);
        if (msg == NULL)
        {
            if (shell_top_active)
            {
                shell_show_top();
            }
            continue;
        }

        if (akos_message_get_pure_data(msg) == AKOS_SHELL_SIG_RX_READY)
        {
            shell_process_rx();
        }

        akos_message_free(msg);
    }
}

void akos_shell_rx_isr(uint8_t byte)
{
    const uint8_t shell_thread_id = akos_thread_get_shell_thread_id();
    const uint8_t next = (uint8_t)((shell_rx_head + 1u) % OS_CFG_SHELL_RX_BUFFER_SIZE);

    if (next != shell_rx_tail)
    {
        shell_rx_buffer[shell_rx_head] = byte;
        shell_rx_head = next;
    }

    if ((shell_thread_id == UINT8_MAX) || shell_rx_notified)
    {
        return;
    }

    shell_rx_notified = true;
    akos_thread_post_msg_pure(shell_thread_id, AKOS_SHELL_SIG_RX_READY);
}

static void shell_process_rx(void)
{
    for (;;)
    {
        uint8_t byte;

        if (shell_rx_tail == shell_rx_head)
        {
            shell_rx_notified = false;
            if (shell_rx_tail == shell_rx_head)
            {
                break;
            }
            shell_rx_notified = true;
        }

        byte = shell_rx_buffer[shell_rx_tail];
        shell_rx_tail = (uint8_t)((shell_rx_tail + 1u) % OS_CFG_SHELL_RX_BUFFER_SIZE);
        shell_process_byte(byte);
    }
}

static void shell_process_byte(uint8_t byte)
{
    if (shell_top_active)
    {
        if ((byte == 'q') || (byte == 'Q') || (byte == 0x03u))
        {
            shell_top_exit();
        }
        return;
    }

    if ((byte == '\r') || (byte == '\n'))
    {
        SYS_PRINT("\r\n");
        shell_line_buffer[shell_line_length] = '\0';
        shell_execute_line();
        shell_line_length = 0u;
        if (!shell_top_active)
        {
            shell_prompt();
        }
        return;
    }

    if (((byte == '\b') || (byte == 0x7Fu)) && (shell_line_length > 0u))
    {
        shell_line_length--;
        shell_line_buffer[shell_line_length] = '\0';
        SYS_PRINT("\b \b");
        return;
    }

    if ((byte < 0x20u) || (byte > 0x7Eu))
    {
        return;
    }

    if (shell_line_length < (OS_CFG_SHELL_LINE_BUFFER_SIZE - 1u))
    {
        shell_line_buffer[shell_line_length++] = (char)byte;
        shell_line_buffer[shell_line_length] = '\0';
        SYS_PRINT("%c", byte);
    }
}

static void shell_execute_line(void)
{
    char *argv[OS_CFG_SHELL_MAX_ARGS];
    int argc;
    int consumed_argc;
    const akos_shell_cmd_t *cmd = __start_shell_cmd_desc;

    if (shell_line_length == 0u)
    {
        return;
    }

    argc = shell_tokenize(shell_line_buffer, argv, (int)OS_CFG_SHELL_MAX_ARGS);
    if (argc <= 0)
    {
        return;
    }

    if (strcmp(argv[0], "help") == 0)
    {
        shell_show_help();
        return;
    }

    if (strcmp(argv[0], "clear") == 0)
    {
        SYS_PRINT("\033[2J\033[H");
        return;
    }

    if (strcmp(argv[0], "top") == 0)
    {
        shell_top_enter();
        return;
    }

    for (; cmd < __stop_shell_cmd_desc; ++cmd)
    {
        if ((cmd->handler != NULL) && shell_match_path(cmd->path, argc, argv, &consumed_argc))
        {
            (void)cmd->handler(argc - consumed_argc, &argv[consumed_argc]);
            return;
        }
    }

    SYS_PRINT("Unknown command: %s\r\n", argv[0]);
}

static int shell_tokenize(char *line, char **argv, int argv_max)
{
    int argc = 0;
    bool in_token = false;

    while ((*line != '\0') && (argc < argv_max))
    {
        if ((*line == ' ') || (*line == '\t'))
        {
            *line = '\0';
            in_token = false;
        }
        else if (!in_token)
        {
            argv[argc++] = line;
            in_token = true;
        }

        line++;
    }

    return argc;
}

static void shell_show_help(void)
{
    const akos_shell_cmd_t *cmd = __start_shell_cmd_desc;

    SYS_PRINT("  %-16s %s\r\n", "help", "Show available commands");
    SYS_PRINT("  %-16s %s\r\n", "clear", "Clear terminal screen");
    SYS_PRINT("  %-16s %s\r\n", "top", "Live MCU load screen, press q to exit");

    for (; cmd < __stop_shell_cmd_desc; ++cmd)
    {
        SYS_PRINT("  %-16s %s\r\n", cmd->path, cmd->help);
    }
}

static void shell_show_top(void)
{
    uint32_t idle_ticks_now;
    uint32_t total_ticks_now;
    uint32_t idle_delta;
    uint32_t total_delta;
    uint8_t cpu_load;
    uint8_t runtime_count;
    uint8_t runtime_limit;
    uint8_t index;
    uint8_t prio;
    uint8_t idle_thread_id;
    uint8_t shell_thread_id;
    uint8_t timer_thread_id;

    akos_thread_get_runtime_totals(&idle_ticks_now, &total_ticks_now);

    SYS_PRINT("\033[2J\033[H");
    SYS_PRINT("AKOS top - press q to exit\r\n\r\n");

    if (!shell_top_has_snapshot)
    {
        shell_top_prev_idle_ticks = idle_ticks_now;
        shell_top_prev_total_ticks = total_ticks_now;
        runtime_count = akos_thread_get_runtime_count();
        runtime_limit = runtime_count;
        for (index = 0u; index < runtime_limit; ++index)
        {
            thread_runtime_snapshot_t snapshot;
            akos_thread_get_runtime_snapshot(index, &snapshot);
            shell_top_prev_thread_ticks[index] = snapshot.run_ticks;
        }
        shell_top_has_snapshot = true;
        SYS_PRINT("cpu load : sampling...\r\n");
        return;
    }

    idle_delta = idle_ticks_now - shell_top_prev_idle_ticks;
    total_delta = total_ticks_now - shell_top_prev_total_ticks;

    shell_top_prev_idle_ticks = idle_ticks_now;
    shell_top_prev_total_ticks = total_ticks_now;

    if (total_delta == 0u)
    {
        cpu_load = 0u;
    }
    else
    {
        uint32_t busy_delta = (idle_delta >= total_delta) ? 0u : (total_delta - idle_delta);
        uint32_t load = (busy_delta * 100u) / total_delta;
        cpu_load = (load > 100u) ? 100u : (uint8_t)load;
    }

    SYS_PRINT("total cpu load : %u%%\r\n\r\n", (unsigned)cpu_load);
    SYS_PRINT("%-5s %-20s %s\r\n", "prio", "name", "load");

    runtime_count = akos_thread_get_runtime_count();
    runtime_limit = runtime_count;
    idle_thread_id = akos_thread_get_idle_thread_id();
    shell_thread_id = akos_thread_get_shell_thread_id();
    timer_thread_id = akos_thread_get_timer_thread_id();
    for (index = 0u; index < runtime_limit; ++index)
    {
        thread_runtime_snapshot_t snapshot;

        akos_thread_get_runtime_snapshot(index, &snapshot);
        if (snapshot.id == UINT8_MAX)
        {
            continue;
        }

        uint32_t thread_delta = snapshot.run_ticks - shell_top_prev_thread_ticks[index];
        shell_top_prev_thread_ticks[index] = snapshot.run_ticks;
        shell_top_thread_prio[index] = snapshot.prio;
        shell_top_thread_name[index] = snapshot.name;
        shell_top_thread_load[index] = (total_delta == 0u) ? 0u : ((thread_delta * 100u) / total_delta);
    }

    for (prio = 0u; prio < OS_CFG_PRIO_MAX; ++prio)
    {
        for (index = 0u; index < runtime_limit; ++index)
        {
            thread_runtime_snapshot_t snapshot;

            akos_thread_get_runtime_snapshot(index, &snapshot);
            if ((snapshot.id == UINT8_MAX) ||
                (snapshot.id == idle_thread_id) ||
                (snapshot.id == shell_thread_id) ||
                (snapshot.id == timer_thread_id) ||
                (snapshot.prio != prio))
            {
                continue;
            }

            SYS_PRINT("%-5u %-20s %lu%%\r\n",
                      (unsigned)shell_top_thread_prio[index],
                      (shell_top_thread_name[index] != NULL) ? shell_top_thread_name[index] : "-",
                      (unsigned long)shell_top_thread_load[index]);
        }
    }

    SYS_PRINT("--------------------------------\r\n");
    for (prio = 0u; prio < OS_CFG_PRIO_MAX; ++prio)
    {
        for (index = 0u; index < runtime_limit; ++index)
        {
            thread_runtime_snapshot_t snapshot;

            akos_thread_get_runtime_snapshot(index, &snapshot);
            if ((snapshot.id == UINT8_MAX) ||
                ((snapshot.id != shell_thread_id) &&
                 (snapshot.id != timer_thread_id) &&
                 (snapshot.id != idle_thread_id)) ||
                (snapshot.prio != prio))
            {
                continue;
            }

            SYS_PRINT("%-5u %-20s %lu%%\r\n",
                      (unsigned)shell_top_thread_prio[index],
                      (shell_top_thread_name[index] != NULL) ? shell_top_thread_name[index] : "-",
                      (unsigned long)shell_top_thread_load[index]);
        }
    }
}

static void shell_top_enter(void)
{
    shell_top_active = true;
    shell_top_has_snapshot = false;
    memset(shell_top_prev_thread_ticks, 0x00, sizeof(shell_top_prev_thread_ticks));
    memset(shell_top_thread_prio, 0x00, sizeof(shell_top_thread_prio));
    memset(shell_top_thread_load, 0x00, sizeof(shell_top_thread_load));
    memset(shell_top_thread_name, 0x00, sizeof(shell_top_thread_name));
    shell_show_top();
}

static void shell_top_exit(void)
{
    shell_top_active = false;
    shell_top_has_snapshot = false;
    SYS_PRINT("\033[2J\033[H");
    shell_prompt();
}

static void shell_prompt(void)
{
    SYS_PRINT(OS_CFG_SHELL_PROMPT_NAME);
}

static bool shell_match_path(const char *path, int argc, char **argv, int *consumed_argc)
{
    int arg_index = 0;

    while (*path != '\0')
    {
        const char *token_start;
        size_t token_len = 0u;

        while (*path == ' ')
        {
            path++;
        }

        if (*path == '\0')
        {
            break;
        }

        if (arg_index >= argc)
        {
            return false;
        }

        token_start = path;
        while ((path[token_len] != '\0') && (path[token_len] != ' '))
        {
            token_len++;
        }

        if ((strlen(argv[arg_index]) != token_len) ||
            (strncmp(argv[arg_index], token_start, token_len) != 0))
        {
            return false;
        }

        arg_index++;
        path += token_len;
    }

    *consumed_argc = arg_index;
    return true;
}

#else

void akos_shell_feature_init(void)
{
}

void akos_shell_thread(void *p_arg)
{
    (void)p_arg;
    for (;;)
    {
    }
}

void akos_shell_rx_isr(uint8_t byte)
{
    (void)byte;
}

#endif
