#ifndef AKOS_SHELL_H
#define AKOS_SHELL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "config.h"

typedef int (*akos_shell_handler_t)(int argc, char **argv);

typedef struct akos_shell_cmd
{
    const char *path;
    const char *help;
    akos_shell_handler_t handler;
} akos_shell_cmd_t;

#define AKOS_SHELL_CMD_DEFINE(_name, _path, _help, _handler)                  \
    const akos_shell_cmd_t _name __attribute__((used, section("shell_cmd_desc"))) = \
    {                                                                         \
        .path = (_path),                                                      \
        .help = (_help),                                                      \
        .handler = (_handler),                                                \
    }

void akos_shell_feature_init(void);
void akos_shell_thread(void *p_arg);
void akos_shell_rx_isr(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* AKOS_SHELL_H */
