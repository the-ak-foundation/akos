/**
 ******************************************************************************
 * @file    oled_i2c.c
 * @brief   Private GPIO I2C transport for the OLED controller
 *
 * @module  Game Hub - OLED I2C
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "oled_i2c.h"

#include "io_cfg.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
#define OLED_I2C_CLOCK_DELAY_LOOPS (20u)
#define OLED_I2C_DATA_DELAY_LOOPS  (1u)

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/
static bool oled_i2c_initializing;

/*==================================================================================================
*                                        PRIVATE FUNCTIONS
==================================================================================================*/
static void oled_i2c_delay(uint32_t delay_loops) {
    for (uint32_t loop = 0u; loop < delay_loops; loop++) {
        __NOP();
    }
}

static void oled_i2c_pin_init(GPIO_TypeDef *port, uint8_t pin) {
    const uint32_t shift = (uint32_t)pin * 2u;
    const uint32_t mask = 1u << pin;

    /* Drive low only; external board pull-ups provide the bus high level. */
    port->MODER = (port->MODER & ~(3u << shift)) | (1u << shift);
    port->OTYPER |= mask;
    port->OSPEEDR = (port->OSPEEDR & ~(3u << shift)) | (1u << shift);
    port->PUPDR &= ~(3u << shift);
    port->BSRR = mask;
}

static void oled_i2c_scl_write(bool high) {
    OLED_CLK_IO_PORT->BSRR = high ? (1u << OLED_CLK_IO_PIN) : (1u << (OLED_CLK_IO_PIN + 16u));
    if (oled_i2c_initializing) {
        oled_i2c_delay(OLED_I2C_CLOCK_DELAY_LOOPS);
    }
}

static void oled_i2c_sda_write(bool high) {
    OLED_DATA_IO_PORT->BSRR = high ? (1u << OLED_DATA_IO_PIN) : (1u << (OLED_DATA_IO_PIN + 16u));
    /* Preserve SDA setup and START/STOP hold time during initialization. */
    if (oled_i2c_initializing) {
        oled_i2c_delay(OLED_I2C_DATA_DELAY_LOOPS);
    }
}

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void oled_i2c_init(void) {
    RCC->AHBENR |= OLED_CLK_IO_CLOCK | OLED_DATA_IO_CLOCK;
    oled_i2c_pin_init(OLED_DATA_IO_PORT, OLED_DATA_IO_PIN);
    oled_i2c_pin_init(OLED_CLK_IO_PORT, OLED_CLK_IO_PIN);
    oled_i2c_initializing = false;
}

void oled_i2c_set_initializing(bool initializing) {
    oled_i2c_initializing = initializing;
}

void oled_i2c_start(void) {
    oled_i2c_scl_write(true);
    oled_i2c_sda_write(true);
    oled_i2c_sda_write(false);
    oled_i2c_scl_write(false);
}

void oled_i2c_stop(void) {
    oled_i2c_scl_write(false);
    oled_i2c_sda_write(false);
    oled_i2c_scl_write(true);
    oled_i2c_sda_write(true);
}

bool oled_i2c_write_byte(uint8_t value) {
    bool acknowledged;

    for (uint8_t bit = 0u; bit < 8u; bit++) {
        oled_i2c_sda_write((value & 0x80u) != 0u);
        oled_i2c_scl_write(true);
        oled_i2c_scl_write(false);
        value <<= 1u;
    }

    /* Release SDA so the OLED can pull it low during the ninth ACK clock. */
    oled_i2c_sda_write(true);
    oled_i2c_scl_write(true);
    acknowledged = (OLED_DATA_IO_PORT->IDR & (1u << OLED_DATA_IO_PIN)) == 0u;
    oled_i2c_scl_write(false);

    return acknowledged;
}
