#include "app.h"

#include "game_hal.h"
#include "game_hub_logo.h"
#include "game_manager.h"
#include "hollow_audio.h"

#define GAME_HUB_SPLASH_MS (3000u)
#define GAME_HUB_LOGO_X \
    ((GAME_DISPLAY_WIDTH - GAME_HUB_LOGO_WIDTH) / 2u)

static volatile bool game_display_paused;

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
    game_display_init();
    game_display_set_power(true);
    hollow_audio_init();
    game_uart_puts("OLED display: INITIALIZED\n");

    game_display_clear();
    game_display_draw_bitmap(GAME_HUB_LOGO_X, 0u, game_hub_logo,
                             GAME_HUB_LOGO_WIDTH, GAME_HUB_LOGO_HEIGHT, true);
    game_display_present();
    akos_thread_delay(GAME_HUB_SPLASH_MS);

    game_manager_init(&manager);
    next_frame = akos_thread_get_tick();

    while (true) {
        uint8_t input_state = 0u;
        uint32_t now;

        if (game_display_paused) {
            akos_thread_delay(10u);
            next_frame = akos_thread_get_tick();
            continue;
        }

        if (game_button_is_pressed(GAME_BUTTON_UP)) {
            input_state |= GAME_INPUT_UP;
        }
        if (game_button_is_pressed(GAME_BUTTON_DOWN)) {
            input_state |= GAME_INPUT_DOWN;
        }
        if (game_button_is_pressed(GAME_BUTTON_MODE)) {
            input_state |= GAME_INPUT_MODE;
        }

        /* The original Arduboy game updates at 60 Hz and renders at 30 FPS. */
        game_manager_update(&manager, input_state, 16u);
        game_manager_update(&manager, input_state, 17u);
        now = akos_thread_get_tick();
        hollow_audio_play_events(game_manager_take_events(&manager), now);
        hollow_audio_update(now);
        game_manager_draw(&manager);
        game_display_present();
        if (first_frame) {
            game_led_write(GAME_LED_2, true);
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
