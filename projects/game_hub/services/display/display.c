/**
 ******************************************************************************
 * @file    display.c
 * @brief   Connect the graphics framebuffer to the OLED driver
 *
 * @module  Game Hub - Display
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "display.h"

#include "graphics.h"
#include "oled.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
_Static_assert(GFX_WIDTH == OLED_WIDTH && GFX_HEIGHT == OLED_HEIGHT,
               "Framebuffer geometry must match the OLED");
_Static_assert(GFX_BUFFER_SIZE == OLED_BUFFER_SIZE, "Framebuffer size must match the OLED");

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
bool display_init(void) {
    bool ready = oled_init();

    gfx_clear();
    return display_present() && ready;
}

bool display_present(void) {
    size_t size;
    const uint8_t *buffer = gfx_get_buffer(&size);

    return oled_present(buffer, size);
}

bool display_set_power(bool on) {
    return oled_set_power(on);
}
