/**
 ******************************************************************************
 * @file    task_game_hub.c
 * @brief   Task game hub module
 *
 * @module  Game Hub - App
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "app.h"

#include "app_dbg.h"
#include "graphics.h"
#include "display.h"
#include "bsp.h"

#include "game_hub_logo.h"
#include "game_manager.h"
#if GAME_HUB_ENABLE_HOLLOW
#include "hollow_audio.h"
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define GAME_HUB_SPLASH_MS (3000u)
#define GAME_HUB_LOGO_X    ((GFX_WIDTH - GAME_HUB_LOGO_WIDTH) / 2u)

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/

static volatile bool game_display_paused;

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void game_app_display_pause(void) {
    game_display_paused = true;
}

void game_app_display_resume(void) {
    game_display_paused = false;
}

void task_game_hub(void *argument) {
    static game_manager_t manager;
    bool first_frame = true;
    uint32_t next_frame;

    (void)argument;

    /* Match the reference application: initialize OLED 100 ms after startup. */
    akos_thread_delay(100u);
    if (display_init()) {
        APP_DBG("OLED bus: READY\n");
    } else {
        APP_DBG("OLED bus: NO ACK\n");
    }
    display_set_power(true);

#if GAME_HUB_ENABLE_HOLLOW
    hollow_audio_init();
#endif
    APP_DBG("OLED display: INITIALIZED\n");

    gfx_clear();
    gfx_draw_bitmap(GAME_HUB_LOGO_X, 0u, game_hub_logo, GAME_HUB_LOGO_WIDTH, GAME_HUB_LOGO_HEIGHT,
                    true);
    display_present();
    akos_thread_delay(GAME_HUB_SPLASH_MS);

    game_manager_init(&manager);
    next_frame = akos_thread_get_tick();

    while (true) {
        uint8_t input_state;
        uint32_t now;

        if (game_display_paused) {
            akos_thread_delay(10u);
            next_frame = akos_thread_get_tick();
            continue;
        }

        input_state = game_input_read();

        /* The original Arduboy game updates at 60 Hz and renders at 30 FPS. */
        game_manager_update(&manager, input_state, 16u);
        game_manager_update(&manager, input_state, 17u);
#if GAME_HUB_ENABLE_HOLLOW
        now = akos_thread_get_tick();
        hollow_audio_play_events(game_manager_take_events(&manager), now);
        hollow_audio_update(now);
#endif
        game_manager_draw(&manager);
        display_present();
        if (first_frame) {
            bsp_led_write(BSP_LED_2, true);
            first_frame = false;
        }
        next_frame += GAME_DISPLAY_PERIOD_TICKS;
        now = akos_thread_get_tick();
        if ((int32_t)(next_frame - now) > 0) {
            akos_thread_delay(next_frame - now);
        } else {
            next_frame = now;
            /*
             * A full software-I2C frame can exceed its budget. Always yield
             * when late so the higher-priority game task cannot starve shell.
             */
            akos_thread_delay(1u);
        }
    }
}
