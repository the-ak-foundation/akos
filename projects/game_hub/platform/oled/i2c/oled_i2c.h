/**
 ******************************************************************************
 * @file    oled_i2c.h
 * @brief   Private GPIO I2C transport for the OLED controller
 *
 * @module  Game Hub - OLED I2C
 ******************************************************************************
 */

#ifndef __OLED_I2C_H__
#define __OLED_I2C_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>
#include <stdint.h>

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
/* Private to platform/oled; application code includes oled.h only. */
void oled_i2c_init(void);
void oled_i2c_set_initializing(bool initializing);
void oled_i2c_start(void);
void oled_i2c_stop(void);
bool oled_i2c_write_byte(uint8_t value);

#endif /* __OLED_I2C_H__ */
