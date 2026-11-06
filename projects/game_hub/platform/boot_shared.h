/**
 ******************************************************************************
 * @file    boot_shared.h
 * @brief   Boot shared module
 *
 * @module  Game Hub - Platform
 ******************************************************************************
 */

#ifndef __BOOT_SHARED_H__
#define __BOOT_SHARED_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef struct {
    uint8_t command;
    uint8_t container;
    uint8_t io_driver;
    uint32_t destination_address;
    uint32_t source_address;
} boot_shared_app_command_t;

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
bool boot_shared_request_uart_update(void);
void boot_shared_get_app_command(boot_shared_app_command_t *command);

#ifdef __cplusplus
}
#endif

#endif /* __BOOT_SHARED_H__ */
