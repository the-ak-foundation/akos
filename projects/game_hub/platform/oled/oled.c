/**
 ******************************************************************************
 * @file    oled.c
 * @brief   SSD1309 and SH1106 monochrome OLED driver
 *
 * @module  Game Hub - OLED
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "oled.h"

#include "i2c/oled_i2c.h"
#include "io_cfg.h"
#include "system_stm32l1xx.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
/* This is the complete address byte including the I2C write bit. */
#define OLED_I2C_ADDRESS (0x78u)

#if defined(SSD1309_DRIVER_EN)
#define OLED_COLUMN_OFFSET (0u)
#elif defined(SH1106_DRIVER_EN)
#define OLED_COLUMN_OFFSET (2u)
#else
#error "Select SSD1309_DRIVER_EN or SH1106_DRIVER_EN"
#endif

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void oled_delay_ms(uint32_t milliseconds) {
    const uint32_t loops_per_ms = SystemCoreClock / 4000u;

    while (milliseconds-- > 0u) {
        for (volatile uint32_t loop = 0u; loop < loops_per_ms; loop++) {
            __NOP();
        }
    }
}

static void oled_hardware_reset(void) {
    const uint32_t shift = OLED_RES_IO_PIN * 2u;
    const uint32_t mask = 1u << OLED_RES_IO_PIN;

    RCC->AHBENR |= OLED_RES_IO_CLOCK;
    OLED_RES_IO_PORT->MODER = (OLED_RES_IO_PORT->MODER & ~(3u << shift)) | (1u << shift);
    OLED_RES_IO_PORT->OTYPER &= ~mask;
    OLED_RES_IO_PORT->OSPEEDR &= ~(3u << shift);
    OLED_RES_IO_PORT->PUPDR &= ~(3u << shift);

    /* SSD1309/SH1106 reset is active low. */
    OLED_RES_IO_PORT->BSRR = mask << 16u;
    oled_delay_ms(100u);
    OLED_RES_IO_PORT->BSRR = mask;
    oled_delay_ms(10u);
}

static bool oled_write_command(uint8_t command) {
    bool acknowledged;

    oled_i2c_start();
    acknowledged = oled_i2c_write_byte(OLED_I2C_ADDRESS);
    acknowledged = oled_i2c_write_byte(0x00u) && acknowledged;
    acknowledged = oled_i2c_write_byte(command) && acknowledged;
    oled_i2c_stop();

    return acknowledged;
}

static bool oled_begin_data(void) {
    bool acknowledged;

    oled_i2c_start();
    acknowledged = oled_i2c_write_byte(OLED_I2C_ADDRESS);
    acknowledged = oled_i2c_write_byte(0x40u) && acknowledged;

    return acknowledged;
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
bool oled_init(void) {
#if defined(SSD1309_DRIVER_EN)
    static const uint8_t init_commands[] = {
        0xAEu, 0x20u, 0x02u, 0x40u, 0xA1u, 0xC8u, 0xA8u, 0x3Fu, 0xD3u, 0x00u, 0xD5u, 0x80u, 0xD9u,
        0xF1u, 0xDAu, 0x12u, 0xDBu, 0x40u, 0x81u, 0xCFu, 0x8Du, 0x14u, 0xA4u, 0xA6u, 0xAFu,
    };
#else
    static const uint8_t init_commands[] = {
        0xAEu, 0xD5u, 0x80u, 0xA8u, 0x3Fu, 0xD3u, 0x00u, 0x40u, 0x8Du, 0x14u, 0x20u, 0x00u, 0xA1u,
        0xC8u, 0xDAu, 0x12u, 0x81u, 0xCFu, 0xD9u, 0xF1u, 0xDBu, 0x40u, 0xA4u, 0xA6u, 0xAFu,
    };
#endif
    bool acknowledged = true;

    oled_i2c_init();
    oled_hardware_reset();
    oled_i2c_set_initializing(true);
    for (size_t index = 0u; index < sizeof(init_commands); index++) {
        acknowledged = oled_write_command(init_commands[index]) && acknowledged;
    }
    oled_delay_ms(10u);
    oled_i2c_set_initializing(false);

    return acknowledged;
}

bool oled_present(const uint8_t *buffer, size_t size) {
    bool acknowledged = true;

    if ((buffer == NULL) || (size != OLED_BUFFER_SIZE)) {
        return false;
    }

    for (uint8_t page = 0u; page < OLED_HEIGHT / 8u; page++) {
        acknowledged = oled_write_command((uint8_t)(0xB0u + page)) && acknowledged;
        acknowledged = oled_write_command(OLED_COLUMN_OFFSET & 0x0Fu) && acknowledged;
        acknowledged =
            oled_write_command((uint8_t)(0x10u | (OLED_COLUMN_OFFSET >> 4u))) && acknowledged;

        /* Both controllers auto-increment columns; transmit a complete page. */
        acknowledged = oled_begin_data() && acknowledged;
        for (uint8_t column = 0u; column < OLED_WIDTH; column++) {
            uint32_t index = (uint32_t)page * OLED_WIDTH + column;

            acknowledged = oled_i2c_write_byte(buffer[index]) && acknowledged;
        }
        oled_i2c_stop();
    }

    return acknowledged;
}

bool oled_set_power(bool on) {
    return oled_write_command(on ? 0xAFu : 0xAEu);
}
