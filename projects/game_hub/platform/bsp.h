/**
 ******************************************************************************
 * @file    bsp.h
 * @brief   Bsp
 *
 * @module  Game Hub - Platform
 ******************************************************************************
 */

#ifndef BSP_H
#define BSP_H

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
typedef enum { BSP_BUTTON_DOWN = 0, BSP_BUTTON_UP, BSP_BUTTON_MODE, BSP_BUTTON_COUNT } bsp_button_t;

typedef enum { BSP_LED_LIFE = 0, BSP_LED_1, BSP_LED_2, BSP_LED_COUNT } bsp_led_t;

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void bsp_init(void);
bool bsp_button_is_pressed(bsp_button_t button);
void bsp_led_write(bsp_led_t led, bool on);
void bsp_led_toggle(bsp_led_t led);

#ifdef __cplusplus
}
#endif

#endif
