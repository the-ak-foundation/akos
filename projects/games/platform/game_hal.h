#ifndef __GAME_HAL_H__
#define __GAME_HAL_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GAME_DISPLAY_WIDTH  (128u)
#define GAME_DISPLAY_HEIGHT (64u)

typedef enum {
    GAME_BUTTON_DOWN = 0,
    GAME_BUTTON_UP,
    GAME_BUTTON_MODE,
    GAME_BUTTON_COUNT
} game_button_t;

typedef enum {
    GAME_LED_LIFE = 0,
    GAME_LED_1,
    GAME_LED_2,
    GAME_LED_COUNT
} game_led_t;

void game_hal_init(void);

bool game_button_is_pressed(game_button_t button);

void game_led_write(game_led_t led, bool on);
void game_led_toggle(game_led_t led);

void game_display_init(void);
void game_display_clear(void);
void game_display_draw_pixel(uint8_t x, uint8_t y, bool on);
void game_display_draw_bitmap(uint8_t x, uint8_t y, const uint8_t *bitmap,
                              uint8_t width, uint8_t height, bool on);
void game_display_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                            bool on);
void game_display_present(void);
void game_display_set_power(bool on);
const uint8_t *game_display_get_buffer(size_t *size);

void game_buzzer_init(void);
void game_buzzer_tone(uint16_t frequency_hz);
void game_buzzer_off(void);

void game_system_reboot(void);

void game_uart_putc(char character);
void game_uart_puts(const char *string);
void game_uart_write(const uint8_t *data, size_t size);
bool game_uart_try_getc(char *character);

#ifdef __cplusplus
}
#endif

#endif /* __GAME_HAL_H__ */
