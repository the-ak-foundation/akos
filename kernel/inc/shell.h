/*!
 * @file	shell.h
 * @author	Snoopy3921 - AK Foundation
 * @date	2026/05/08
 * @module	AKOS
 * @brief	Shell command interface APIs.
 */

#ifndef SHELL_H
#define SHELL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "config.h"

/**
 * @brief Shell command handler function signature.
 * @param argc Number of command arguments.
 * @param argv Command argument vector.
 * @return Command execution status.
 */
typedef int (*akos_shell_handler_t)(int argc, char **argv);

/**
 * @struct akos_shell_cmd
 * @brief Shell command descriptor stored in the command table.
 */
typedef struct akos_shell_cmd
{
    const char *path;                       /**< Command path/name. */
    const char *help;                       /**< Command help text. */
    akos_shell_handler_t handler;           /**< Function called when command runs. */
} akos_shell_cmd_t;

/**
 * @brief Define and register a shell command descriptor.
 * @param _name Descriptor symbol name.
 * @param _path Command path/name.
 * @param _help Command help text.
 * @param _handler Command handler function.
 */
#define AKOS_SHELL_CMD_DEFINE(_name, _path, _help, _handler)    \
    const akos_shell_cmd_t _name                                \
    __attribute__((used, section("shell_cmd_desc"))) =          \
    {                                                           \
        .path = (_path),                                        \
        .help = (_help),                                        \
        .handler = (_handler),                                  \
    }

/**
 * @brief Initialize optional shell feature resources.
 */
void akos_shell_feature_init(void);

/**
 * @brief Shell thread entry function.
 * @param p_arg Thread argument pointer.
 */
void akos_shell_thread(void *p_arg);

/**
 * @brief Feed one received byte into the shell from an interrupt context.
 * @param byte Received byte.
 */
void akos_shell_rx_isr(uint8_t byte);

#ifdef __cplusplus
}
#endif
#endif /* SHELL_H */
