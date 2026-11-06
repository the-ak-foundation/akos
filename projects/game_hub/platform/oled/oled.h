/**
 ******************************************************************************
 * @file    oled.h
 * @brief   SSD1309 and SH1106 monochrome OLED driver
 *
 * @module  Game Hub - OLED
 ******************************************************************************
 */

#ifndef __OLED_H__
#define __OLED_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
#define OLED_WIDTH       (128u)
#define OLED_HEIGHT      (64u)
#define OLED_BUFFER_SIZE (OLED_WIDTH * OLED_HEIGHT / 8u)

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
/** Initialize the controller without allocating or clearing a framebuffer. */
bool oled_init(void);

/**
 * Send a complete framebuffer and return whether all bytes were acknowledged.
 * Buffer layout: buffer[(y / 8u) * OLED_WIDTH + x], bit (y & 7u).
 * The caller retains ownership; size must equal OLED_BUFFER_SIZE.
 */
bool oled_present(const uint8_t *buffer, size_t size);
bool oled_set_power(bool on);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H__ */
