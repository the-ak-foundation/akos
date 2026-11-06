/**
 ******************************************************************************
 * @file    boot_shared.c
 * @brief   Boot shared module
 *
 * @module  Game Hub - Platform
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "boot_shared.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stm32l1xx.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define BOOT_SHARED_ADDRESS         (0x08002000u)
#define BOOT_APPLICATION_ADDRESS    (0x08003000u)
#define BOOT_COMMAND_UPDATE_REQUEST (0x02u)
#define BOOT_CONTAINER_DIRECTLY     (0x01u)
#define BOOT_IO_DRIVER_UART         (0x02u)
#define BOOT_FLASH_TIMEOUT          (1000000u)
#define BOOT_FLASH_PEKEY1           (0x89ABCDEFu)
#define BOOT_FLASH_PEKEY2           (0x02030405u)
#define BOOT_FLASH_PRGKEY1          (0x8C9DAEBFu)
#define BOOT_FLASH_PRGKEY2          (0x13141516u)
/* STM32L1 StdPeriph FLASH_GetStatus() treats SR[12:8] as errors. Bit 12
   (OPTVERRUSR) is absent from the reduced CMSIS header used by this project. */
#define BOOT_FLASH_ERROR_MASK (0x00001F00u)

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef struct {
    uint32_t psk;
    uint32_t bin_len;
    uint16_t checksum;
} boot_firmware_header_t;

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t source_task_id;
    uint8_t destination_task_id;
    uint8_t signal;
    uint8_t source_interface_type;
    uint8_t destination_interface_type;
} boot_host_response_t;

typedef struct {
    uint8_t command;
    uint8_t container;
    uint8_t io_driver;
    uint32_t destination_address;
    uint32_t source_address;
    boot_host_response_t host_response;
} boot_firmware_command_t;

typedef struct {
    boot_firmware_header_t current_boot_header;
    boot_firmware_header_t current_app_header;
    boot_firmware_header_t update_boot_header;
    boot_firmware_header_t update_app_header;
    boot_firmware_command_t boot_command;
    boot_firmware_command_t app_command;
} boot_shared_data_t;

_Static_assert(sizeof(boot_firmware_header_t) == 12u, "Boot firmware header ABI mismatch");
_Static_assert(sizeof(boot_firmware_command_t) == 20u, "Boot firmware command ABI mismatch");
_Static_assert(sizeof(boot_shared_data_t) == 88u, "Boot share ABI mismatch");
_Static_assert(offsetof(boot_shared_data_t, app_command) == 68u,
               "Boot app command offset mismatch");

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static bool boot_flash_wait(void) {
    uint32_t timeout = BOOT_FLASH_TIMEOUT;

    while (((FLASH->SR & FLASH_SR_BSY) != 0u) && (timeout > 0u)) {
        timeout--;
    }
    return (timeout > 0u) && ((FLASH->SR & BOOT_FLASH_ERROR_MASK) == 0u);
}

static bool boot_flash_unlock(void) {
    if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0u) {
        FLASH->PEKEYR = BOOT_FLASH_PEKEY1;
        FLASH->PEKEYR = BOOT_FLASH_PEKEY2;
    }
    if ((FLASH->PECR & FLASH_PECR_PRGLOCK) != 0u) {
        FLASH->PRGKEYR = BOOT_FLASH_PRGKEY1;
        FLASH->PRGKEYR = BOOT_FLASH_PRGKEY2;
    }
    FLASH->SR = BOOT_FLASH_ERROR_MASK | FLASH_SR_EOP;
    return (FLASH->PECR & (FLASH_PECR_PELOCK | FLASH_PECR_PRGLOCK)) == 0u;
}

static void boot_flash_lock(void) {
    FLASH->PECR |= FLASH_PECR_PRGLOCK | FLASH_PECR_PELOCK;
}

static bool boot_flash_erase_page(void) {
    if (!boot_flash_wait()) {
        return false;
    }
    FLASH->PECR |= FLASH_PECR_ERASE | FLASH_PECR_PROG;
    *(volatile uint32_t *)(uintptr_t)BOOT_SHARED_ADDRESS = 0u;
    if (!boot_flash_wait()) {
        FLASH->PECR &= ~(FLASH_PECR_ERASE | FLASH_PECR_PROG);
        return false;
    }
    FLASH->PECR &= ~(FLASH_PECR_ERASE | FLASH_PECR_PROG);
    return true;
}

static bool boot_flash_write(const uint8_t *data, size_t size) {
    for (size_t offset = 0u; offset < size; offset += sizeof(uint32_t)) {
        uint32_t word = UINT32_MAX;
        size_t remaining = size - offset;
        size_t copy_size = (remaining < sizeof(word)) ? remaining : sizeof(word);

        memcpy(&word, &data[offset], copy_size);
        *(volatile uint32_t *)(uintptr_t)(BOOT_SHARED_ADDRESS + offset) = word;
        if (!boot_flash_wait()) {
            return false;
        }
    }
    return true;
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
bool boot_shared_request_uart_update(void) {
    boot_shared_data_t data;
    uint32_t interrupt_state = __get_PRIMASK();
    bool success;

    memcpy(&data, (const void *)(uintptr_t)BOOT_SHARED_ADDRESS, sizeof(data));
    data.app_command.command = BOOT_COMMAND_UPDATE_REQUEST;
    data.app_command.container = BOOT_CONTAINER_DIRECTLY;
    data.app_command.io_driver = BOOT_IO_DRIVER_UART;
    data.app_command.destination_address = BOOT_APPLICATION_ADDRESS;
    data.app_command.source_address = 0u;
    memset(&data.app_command.host_response, 0, sizeof(data.app_command.host_response));

    __disable_irq();
    success = boot_flash_unlock() && boot_flash_erase_page() &&
              boot_flash_write((const uint8_t *)&data, sizeof(data));
    boot_flash_lock();
    __DSB();
    __ISB();
    if (interrupt_state == 0u) {
        __enable_irq();
    }

    return success &&
           (memcmp(&data, (const void *)(uintptr_t)BOOT_SHARED_ADDRESS, sizeof(data)) == 0);
}

void boot_shared_get_app_command(boot_shared_app_command_t *command) {
    const boot_shared_data_t *data = (const boot_shared_data_t *)(uintptr_t)BOOT_SHARED_ADDRESS;

    if (command == NULL) {
        return;
    }
    command->command = data->app_command.command;
    command->container = data->app_command.container;
    command->io_driver = data->app_command.io_driver;
    command->destination_address = data->app_command.destination_address;
    command->source_address = data->app_command.source_address;
}
