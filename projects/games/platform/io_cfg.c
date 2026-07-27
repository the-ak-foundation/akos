#include "io_cfg.h"

#include "system_stm32l1xx.h"

static void io_input_pull_up_init(GPIO_TypeDef *port, uint8_t pin) {
    uint32_t shift = (uint32_t)pin * 2u;

    port->MODER &= ~(3u << shift);
    port->PUPDR = (port->PUPDR & ~(3u << shift)) | (1u << shift);
}

static void io_output_init(GPIO_TypeDef *port, uint8_t pin) {
    uint32_t shift = (uint32_t)pin * 2u;
    uint32_t mask  = 1u << pin;

    port->MODER = (port->MODER & ~(3u << shift)) | (1u << shift);
    port->OTYPER &= ~mask;
    port->OSPEEDR &= ~(3u << shift);
    port->PUPDR &= ~(3u << shift);
}

static void io_pin_write(GPIO_TypeDef *port, uint8_t pin, uint8_t high) {
    uint32_t mask = 1u << pin;

    port->BSRR = high != 0u ? mask : (mask << 16u);
}

static int io_pin_read(GPIO_TypeDef *port, uint8_t pin) {
    return (port->IDR & (1u << pin)) != 0u;
}

void timer_delay_init(void) {
    /* The AKOS board clock is already initialized before the application. */
}

void delay(uint32_t milliseconds) {
    uint32_t loops_per_ms = SystemCoreClock / 4000u;

    while (milliseconds-- > 0u) {
        for (volatile uint32_t loop = 0u; loop < loops_per_ms; loop++) {
            __NOP();
        }
    }
}

void io_button_mode_init(void) {
    RCC->AHBENR |= BUTTON_MODE_IO_CLOCK;
    io_input_pull_up_init(BUTTON_MODE_IO_PORT, BUTTON_MODE_IO_PIN);
}

void io_button_up_init(void) {
    RCC->AHBENR |= BUTTON_UP_IO_CLOCK;
    io_input_pull_up_init(BUTTON_UP_IO_PORT, BUTTON_UP_IO_PIN);
}

void io_button_down_init(void) {
    RCC->AHBENR |= BUTTON_DOWN_IO_CLOCK;
    io_input_pull_up_init(BUTTON_DOWN_IO_PORT, BUTTON_DOWN_IO_PIN);
}

uint8_t io_button_mode_read(void) {
    return (uint8_t)io_pin_read(BUTTON_MODE_IO_PORT, BUTTON_MODE_IO_PIN);
}

uint8_t io_button_up_read(void) {
    return (uint8_t)io_pin_read(BUTTON_UP_IO_PORT, BUTTON_UP_IO_PIN);
}

uint8_t io_button_down_read(void) {
    return (uint8_t)io_pin_read(BUTTON_DOWN_IO_PORT, BUTTON_DOWN_IO_PIN);
}

void led_life_init(void) {
    RCC->AHBENR |= LED_LIFE_IO_CLOCK;
    io_output_init(LED_LIFE_IO_PORT, LED_LIFE_IO_PIN);
    led_life_off();
}

void led_life_on(void) {
    io_pin_write(LED_LIFE_IO_PORT, LED_LIFE_IO_PIN, 1u);
}

void led_life_off(void) {
    io_pin_write(LED_LIFE_IO_PORT, LED_LIFE_IO_PIN, 0u);
}

void led_life_toggle(void) {
    io_pin_write(LED_LIFE_IO_PORT, LED_LIFE_IO_PIN,
                 (uint8_t)!io_pin_read(LED_LIFE_IO_PORT, LED_LIFE_IO_PIN));
}

#define IO_DEFINE_OLED_PIN(prefix, port, pin, clock)                         \
    void prefix##_input_mode(void) {                                         \
        RCC->AHBENR |= (clock);                                               \
        io_input_pull_up_init((port), (pin));                                 \
    }                                                                         \
    void prefix##_output_mode(void) {                                        \
        RCC->AHBENR |= (clock);                                               \
        io_output_init((port), (pin));                                        \
    }                                                                         \
    void prefix##_digital_write_low(void) { io_pin_write((port), (pin), 0u); } \
    void prefix##_digital_write_high(void) { io_pin_write((port), (pin), 1u); } \
    int prefix##_digital_read(void) { return io_pin_read((port), (pin)); }

IO_DEFINE_OLED_PIN(oled_clk, OLED_CLK_IO_PORT, OLED_CLK_IO_PIN,
                   OLED_CLK_IO_CLOCK)
IO_DEFINE_OLED_PIN(oled_data, OLED_DATA_IO_PORT, OLED_DATA_IO_PIN,
                   OLED_DATA_IO_CLOCK)
IO_DEFINE_OLED_PIN(oled_res, OLED_RES_IO_PORT, OLED_RES_IO_PIN,
                   OLED_RES_IO_CLOCK)
