/**
 ******************************************************************************
 * @file    app.c
 * @brief   App module
 *
 * @module  Game Hub - App
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "app.h"

#include "app_dbg.h"
#include "bsp.h"

#include <stdbool.h>

#include "system.h"
#include "task_list.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define NOTE "GAME_HUB"

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
int main(void) {
    bsp_init();
    bsp_led_write(BSP_LED_1, true);
    system_init_console();

#if defined(SSD1309_DRIVER_EN)
    APP_DBG("OLED driver: SSD1309\n");
#else
    APP_DBG("OLED driver: SH1106\n");
#endif
    APP_DBG("Game Hub starting\r\n");
    APP_DBG("[%s]\tInit BSP\r\n", NOTE);
    akos_core_init();
    APP_DBG("[%s]\tInit AKOS v%u.%u.%u\r\n", NOTE, AKOS_VERSION_MAIN, AKOS_VERSION_SUB1,
            AKOS_VERSION_SUB2);
    APP_DBG("[%s]\tProject: Game Hub\r\n", NOTE);
    APP_DBG("[%s]\tStart scheduler\r\n\r\n", NOTE);
    akos_core_run();

    while (true) {
    }
}
