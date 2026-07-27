#ifndef __BOOT_SHARED_H__
#define __BOOT_SHARED_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t command;
    uint8_t container;
    uint8_t io_driver;
    uint32_t destination_address;
    uint32_t source_address;
} boot_shared_app_command_t;

bool boot_shared_request_uart_update(void);
void boot_shared_get_app_command(boot_shared_app_command_t *command);

#endif /* __BOOT_SHARED_H__ */
