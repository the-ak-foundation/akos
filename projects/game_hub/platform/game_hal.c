#include "game_hal.h"

#include <string.h>

#include "io_cfg.h"
#include "stm32l1xx.h"
#include "system_stm32l1xx.h"
#include "xprintf.h"

#define GAME_UART_BAUDRATE (115200u)
#define GAME_SYSTEM_CLOCK_HZ (32000000u)
#define GAME_BUZZER_DUTY_DIVISOR (4u)
#define GAME_OLED_CLOCK_DELAY_LOOPS (20u)
#define GAME_OLED_DATA_DELAY_LOOPS  (1u)
#define GAME_UART_RX_BUFFER_SIZE     (64u)
#define GAME_UART_TX_BUFFER_SIZE     (128u)

/* Buttons are active low: DOWN PB3, UP PC13, MODE PB4. */
typedef struct {
    GPIO_TypeDef *port;
    uint8_t pin;
} game_gpio_t;

static const game_gpio_t game_buttons[GAME_BUTTON_COUNT] = {
    [GAME_BUTTON_DOWN] = {BUTTON_DOWN_IO_PORT, BUTTON_DOWN_IO_PIN},
    [GAME_BUTTON_UP]   = {BUTTON_UP_IO_PORT, BUTTON_UP_IO_PIN},
    [GAME_BUTTON_MODE] = {BUTTON_MODE_IO_PORT, BUTTON_MODE_IO_PIN},
};

/* Board LEDs are active high: life PB8, LED1 PA2, LED2 PA3. */
static const game_gpio_t game_leds[GAME_LED_COUNT] = {
    [GAME_LED_LIFE] = {LED_LIFE_IO_PORT, LED_LIFE_IO_PIN},
    [GAME_LED_1]    = {LED_STATUS_1_IO_PORT, LED_STATUS_1_IO_PIN},
    [GAME_LED_2]    = {LED_STATUS_2_IO_PORT, LED_STATUS_2_IO_PIN},
};

/* SSD1309/SH1106 software-I2C display: SCL PB13, SDA PB12, reset PA15. */
#define GAME_OLED_SCL_PORT OLED_CLK_IO_PORT
#define GAME_OLED_SCL_PIN  OLED_CLK_IO_PIN
#define GAME_OLED_SDA_PORT OLED_DATA_IO_PORT
#define GAME_OLED_SDA_PIN  OLED_DATA_IO_PIN
#define GAME_OLED_RES_PORT OLED_RES_IO_PORT
#define GAME_OLED_RES_PIN  OLED_RES_IO_PIN
#define GAME_OLED_ADDRESS  (0x78u)
#if defined(SSD1309_DRIVER_EN)
#define GAME_OLED_COLUMN_OFFSET (0u)
#elif defined(SH1106_DRIVER_EN)
#define GAME_OLED_COLUMN_OFFSET (2u)
#else
#error "Select SSD1309_DRIVER_EN or SH1106_DRIVER_EN"
#endif
#define GAME_DISPLAY_BUFFER_SIZE \
    (GAME_DISPLAY_WIDTH * GAME_DISPLAY_HEIGHT / 8u)

static uint8_t game_display_buffer[GAME_DISPLAY_BUFFER_SIZE];
static bool game_oled_bus_ready;
static bool game_oled_initializing;
static volatile uint8_t game_uart_rx_buffer[GAME_UART_RX_BUFFER_SIZE];
static volatile uint8_t game_uart_rx_head;
static volatile uint8_t game_uart_rx_tail;
static volatile uint8_t game_uart_tx_buffer[GAME_UART_TX_BUFFER_SIZE];
static volatile uint8_t game_uart_tx_head;
static volatile uint8_t game_uart_tx_tail;

static void game_oled_delay(uint32_t delay_loops) {
    for (uint32_t loop = 0u; loop < delay_loops; loop++) {
        __NOP();
    }
}

static void game_xprintf_putc(int character) {
    game_uart_putc((char)character);
}

static void game_uart_init(void) {
    uint32_t tx_shift = UART1_TX_IO_PIN * 2u;
    uint32_t rx_shift = UART1_RX_IO_PIN * 2u;
    uint32_t tx_af_shift = (UART1_TX_IO_PIN - 8u) * 4u;
    uint32_t rx_af_shift = (UART1_RX_IO_PIN - 8u) * 4u;
    uint32_t mode_mask = (3u << tx_shift) | (3u << rx_shift);

    RCC->AHBENR |= UART1_IO_CLOCK;
    RCC->APB2ENR |= UART1_CLOCK;

    UART1_IO_PORT->MODER =
        (UART1_IO_PORT->MODER & ~mode_mask) |
        (2u << tx_shift) | (2u << rx_shift);
    UART1_IO_PORT->OTYPER &= ~((1u << UART1_TX_IO_PIN) |
                               (1u << UART1_RX_IO_PIN));
    UART1_IO_PORT->PUPDR =
        (UART1_IO_PORT->PUPDR & ~mode_mask) |
        (1u << tx_shift) | (1u << rx_shift);
    UART1_IO_PORT->AFR[1] =
        (UART1_IO_PORT->AFR[1] &
         ~((0xFu << tx_af_shift) | (0xFu << rx_af_shift))) |
        (UART1_AF << tx_af_shift) | (UART1_AF << rx_af_shift);

    USART1->CR1 = 0u;
    USART1->CR2 = 0u;
    USART1->CR3 = 0u;
    USART1->BRR = (SystemCoreClock + GAME_UART_BAUDRATE / 2u) /
                  GAME_UART_BAUDRATE;
    game_uart_rx_head = 0u;
    game_uart_rx_tail = 0u;
    game_uart_tx_head = 0u;
    game_uart_tx_tail = 0u;
    NVIC_SetPriority(USART1_IRQn, 5u);
    NVIC_EnableIRQ(USART1_IRQn);
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE |
                  USART_CR1_UE;
}

void USART1_IRQHandler(void) {
    uint32_t status = USART1->SR;

    if ((status & (USART_SR_RXNE | USART_SR_ORE)) != 0u) {
        uint8_t data = (uint8_t)USART1->DR;

        if ((status & USART_SR_RXNE) != 0u) {
            uint8_t next =
                (uint8_t)((game_uart_rx_head + 1u) %
                          GAME_UART_RX_BUFFER_SIZE);

            if (next != game_uart_rx_tail) {
                game_uart_rx_buffer[game_uart_rx_head] = data;
                game_uart_rx_head = next;
            }
        }
    }

    if (((status & USART_SR_TXE) != 0u) &&
        ((USART1->CR1 & USART_CR1_TXEIE) != 0u)) {
        if (game_uart_tx_tail != game_uart_tx_head) {
            USART1->DR = game_uart_tx_buffer[game_uart_tx_tail];
            game_uart_tx_tail =
                (uint8_t)((game_uart_tx_tail + 1u) %
                          GAME_UART_TX_BUFFER_SIZE);
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}

static void game_gpio_input_pull_up_init(GPIO_TypeDef *port, uint8_t pin) {
    const uint32_t shift = (uint32_t)pin * 2u;

    port->MODER &= ~(3u << shift);
    port->PUPDR = (port->PUPDR & ~(3u << shift)) | (1u << shift);
}

static void game_gpio_output_init(GPIO_TypeDef *port, uint8_t pin, bool high) {
    const uint32_t shift = (uint32_t)pin * 2u;
    const uint32_t mask  = 1u << pin;

    port->MODER = (port->MODER & ~(3u << shift)) | (1u << shift);
    port->OTYPER &= ~mask;
    port->OSPEEDR &= ~(3u << shift);
    port->PUPDR &= ~(3u << shift);
    port->BSRR = high ? mask : (mask << 16u);
}

static void game_gpio_oled_bus_init(GPIO_TypeDef *port, uint8_t pin) {
    const uint32_t shift = (uint32_t)pin * 2u;
    const uint32_t mask = 1u << pin;

    /*
     * I2C lines only drive low. Writing a high level releases the pin and lets
     * the external board pull-up produce the bus high level.
     */
    port->MODER = (port->MODER & ~(3u << shift)) | (1u << shift);
    port->OTYPER |= mask;
    port->OSPEEDR = (port->OSPEEDR & ~(3u << shift)) | (1u << shift);
    port->PUPDR &= ~(3u << shift);
    port->BSRR = mask;
}

static void game_delay_ms(uint32_t milliseconds) {
    const uint32_t loops_per_ms = SystemCoreClock / 4000u;

    while (milliseconds-- > 0u) {
        for (volatile uint32_t loop = 0u; loop < loops_per_ms; loop++) {
            __NOP();
        }
    }
}

static void game_oled_scl_write(bool high) {
    GAME_OLED_SCL_PORT->BSRR = high ? (1u << GAME_OLED_SCL_PIN)
                                     : (1u << (GAME_OLED_SCL_PIN + 16u));
    if (game_oled_initializing) {
        game_oled_delay(GAME_OLED_CLOCK_DELAY_LOOPS);
    }
}

static void game_oled_sda_write(bool high) {
    GAME_OLED_SDA_PORT->BSRR = high ? (1u << GAME_OLED_SDA_PIN)
                                     : (1u << (GAME_OLED_SDA_PIN + 16u));
    /*
     * Preserve SDA setup and START/STOP hold time before SCL changes.
     * The reference digitalWrite implementation supplied this call overhead.
     */
    if (game_oled_initializing) {
        game_oled_delay(GAME_OLED_DATA_DELAY_LOOPS);
    }
}

static void game_oled_start(void) {
    game_oled_scl_write(true);
    game_oled_sda_write(true);
    game_oled_sda_write(false);
    game_oled_scl_write(false);
}

static void game_oled_stop(void) {
    game_oled_scl_write(false);
    game_oled_sda_write(false);
    game_oled_scl_write(true);
    game_oled_sda_write(true);
}

static bool game_oled_write_byte(uint8_t value) {
    bool acknowledged;

    for (uint8_t bit = 0u; bit < 8u; bit++) {
        game_oled_sda_write((value & 0x80u) != 0u);
        game_oled_scl_write(true);
        game_oled_scl_write(false);
        value <<= 1u;
    }

    /* Release SDA so the OLED can pull it low during the ninth ACK clock. */
    game_oled_sda_write(true);
    game_oled_scl_write(true);
    acknowledged = (GAME_OLED_SDA_PORT->IDR &
                    (1u << GAME_OLED_SDA_PIN)) == 0u;
    game_oled_scl_write(false);

    return acknowledged;
}

static bool game_oled_write_command(uint8_t command) {
    bool acknowledged;

    game_oled_start();
    acknowledged = game_oled_write_byte(GAME_OLED_ADDRESS);
    acknowledged = game_oled_write_byte(0x00u) && acknowledged;
    acknowledged = game_oled_write_byte(command) && acknowledged;
    game_oled_stop();

    return acknowledged;
}

static bool game_oled_begin_data(void) {
    bool acknowledged;

    game_oled_start();
    acknowledged = game_oled_write_byte(GAME_OLED_ADDRESS);
    acknowledged = game_oled_write_byte(0x40u) && acknowledged;

    return acknowledged;
}

static void game_oled_hardware_reset(void) {
    const uint32_t reset_mask = 1u << GAME_OLED_RES_PIN;

    /* SSD1309/SH1106 reset is active low. */
    GAME_OLED_RES_PORT->BSRR = reset_mask << 16u;
    game_delay_ms(100u);
    GAME_OLED_RES_PORT->BSRR = reset_mask;
    game_delay_ms(10u);
}

void game_hal_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN |
                   RCC_AHBENR_GPIOCEN;

    for (uint32_t led = 0u; led < GAME_LED_COUNT; led++) {
        game_gpio_output_init(game_leds[led].port, game_leds[led].pin, false);
    }

    for (uint32_t button = 0u; button < GAME_BUTTON_COUNT; button++) {
        game_gpio_input_pull_up_init(game_buttons[button].port,
                                     game_buttons[button].pin);
    }

    game_uart_init();
    xdev_out(game_xprintf_putc);
#if defined(SSD1309_DRIVER_EN)
    xputs("OLED driver: SSD1309\n");
#else
    xputs("OLED driver: SH1106\n");
#endif
}

bool game_button_is_pressed(game_button_t button) {
    if ((uint32_t)button >= GAME_BUTTON_COUNT) {
        return false;
    }

    return (game_buttons[button].port->IDR &
            (1u << game_buttons[button].pin)) == 0u;
}

void game_led_write(game_led_t led, bool on) {
    uint32_t mask;

    if ((uint32_t)led >= GAME_LED_COUNT) {
        return;
    }

    mask = 1u << game_leds[led].pin;
    game_leds[led].port->BSRR = on ? mask : (mask << 16u);
}

void game_led_toggle(game_led_t led) {
    bool is_on;

    if ((uint32_t)led >= GAME_LED_COUNT) {
        return;
    }

    is_on = (game_leds[led].port->ODR & (1u << game_leds[led].pin)) != 0u;
    game_led_write(led, !is_on);
}

void game_display_init(void) {
#if defined(SSD1309_DRIVER_EN)
    static const uint8_t init_commands[] = {
        0xAEu, 0x20u, 0x02u, 0x40u, 0xA1u, 0xC8u, 0xA8u, 0x3Fu,
        0xD3u, 0x00u, 0xD5u, 0x80u, 0xD9u, 0xF1u, 0xDAu, 0x12u,
        0xDBu, 0x40u, 0x81u, 0xCFu, 0x8Du, 0x14u, 0xA4u, 0xA6u,
        0xAFu,
    };
#else
    static const uint8_t init_commands[] = {
        0xAEu, 0xD5u, 0x80u, 0xA8u, 0x3Fu, 0xD3u, 0x00u, 0x40u,
        0x8Du, 0x14u, 0x20u, 0x00u, 0xA1u, 0xC8u, 0xDAu, 0x12u,
        0x81u, 0xCFu, 0xD9u, 0xF1u, 0xDBu, 0x40u, 0xA4u, 0xA6u,
        0xAFu,
    };
#endif

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;
    game_gpio_oled_bus_init(GAME_OLED_SDA_PORT, GAME_OLED_SDA_PIN);
    game_gpio_oled_bus_init(GAME_OLED_SCL_PORT, GAME_OLED_SCL_PIN);
    game_gpio_output_init(GAME_OLED_RES_PORT, GAME_OLED_RES_PIN, false);
    game_oled_hardware_reset();

    game_oled_initializing = true;
    game_oled_bus_ready = true;
    for (size_t index = 0u; index < sizeof(init_commands); index++) {
        game_oled_bus_ready =
            game_oled_write_command(init_commands[index]) &&
            game_oled_bus_ready;
    }

    game_delay_ms(10u);
    game_oled_initializing = false;
    game_display_clear();
    game_display_present();
    xputs(game_oled_bus_ready ? "OLED bus: READY\n" :
                                "OLED bus: NO ACK\n");
}

void game_display_clear(void) {
    memset(game_display_buffer, 0, sizeof(game_display_buffer));
}

void game_display_draw_pixel(uint8_t x, uint8_t y, bool on) {
    uint8_t *pixel;
    uint8_t mask;

    if ((x >= GAME_DISPLAY_WIDTH) || (y >= GAME_DISPLAY_HEIGHT)) {
        return;
    }

    pixel = &game_display_buffer[((uint32_t)y / 8u) * GAME_DISPLAY_WIDTH + x];
    mask  = (uint8_t)(1u << (y & 7u));
    *pixel = on ? (uint8_t)(*pixel | mask) : (uint8_t)(*pixel & ~mask);
}

void game_display_draw_bitmap(uint8_t x, uint8_t y, const uint8_t *bitmap,
                              uint8_t width, uint8_t height, bool on) {
    if (bitmap == NULL) {
        return;
    }

    for (uint8_t row = 0u; row < height; row++) {
        for (uint8_t column = 0u; column < width; column++) {
            uint16_t draw_x = (uint16_t)x + column;
            uint16_t draw_y = (uint16_t)y + row;
            uint8_t data = bitmap[(uint32_t)(row / 8u) * width + column];

            if ((draw_x < GAME_DISPLAY_WIDTH) &&
                (draw_y < GAME_DISPLAY_HEIGHT) &&
                ((data & (1u << (row & 7u))) != 0u)) {
                game_display_draw_pixel((uint8_t)draw_x,
                                        (uint8_t)draw_y, on);
            }
        }
    }
}

void game_display_fill_rect(uint8_t x, uint8_t y, uint8_t width,
                            uint8_t height, bool on) {
    for (uint16_t row = y; (row < (uint16_t)y + height) &&
                           (row < GAME_DISPLAY_HEIGHT); row++) {
        for (uint16_t column = x; (column < (uint16_t)x + width) &&
                                  (column < GAME_DISPLAY_WIDTH); column++) {
            game_display_draw_pixel((uint8_t)column, (uint8_t)row, on);
        }
    }
}

void game_display_present(void) {
    for (uint8_t page = 0u; page < 8u; page++) {
        game_oled_bus_ready =
            game_oled_write_command((uint8_t)(0xB0u + page)) &&
            game_oled_bus_ready;
        game_oled_bus_ready =
            game_oled_write_command(GAME_OLED_COLUMN_OFFSET & 0x0Fu) &&
            game_oled_bus_ready;
        game_oled_bus_ready =
            game_oled_write_command((uint8_t)(0x10u |
                                    (GAME_OLED_COLUMN_OFFSET >> 4u))) &&
            game_oled_bus_ready;

        /*
         * Both controllers auto-increment the column in page mode. Keep the
         * complete 128-byte row in one transaction to avoid repeated address
         * and control bytes.
         */
        game_oled_bus_ready =
            game_oled_begin_data() && game_oled_bus_ready;
        for (uint8_t column = 0u; column < GAME_DISPLAY_WIDTH; column++) {
            uint32_t index = (uint32_t)page * GAME_DISPLAY_WIDTH + column;

            game_oled_bus_ready =
                game_oled_write_byte(game_display_buffer[index]) &&
                game_oled_bus_ready;
        }
        game_oled_stop();
    }
}

void game_display_set_power(bool on) {
    game_oled_bus_ready =
        game_oled_write_command(on ? 0xAFu : 0xAEu) &&
        game_oled_bus_ready;
}

const uint8_t *game_display_get_buffer(size_t *size) {
    if (size != NULL) {
        *size = sizeof(game_display_buffer);
    }
    return game_display_buffer;
}

void game_buzzer_init(void) {
    uint32_t shift = BUZZER_IO_PIN * 2u;

    RCC->AHBENR |= BUZZER_IO_CLOCK;
    RCC->APB1ENR |= BUZZER_TIM_CLOCK;

    BUZZER_IO_PORT->MODER =
        (BUZZER_IO_PORT->MODER & ~(3u << shift)) | (2u << shift);
    BUZZER_IO_PORT->OTYPER &= ~(1u << BUZZER_IO_PIN);
    BUZZER_IO_PORT->PUPDR &= ~(3u << shift);
    BUZZER_IO_PORT->AFR[0] =
        (BUZZER_IO_PORT->AFR[0] & ~(0xFu << (BUZZER_IO_PIN * 4u))) |
        (BUZZER_IO_AF << (BUZZER_IO_PIN * 4u));

    BUZZER_TIM->CR1 = 0u;
    BUZZER_TIM->PSC = 31u;
    BUZZER_TIM->CCMR2 = (6u << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;
    BUZZER_TIM->CCER = TIM_CCER_CC3E;
    BUZZER_TIM->EGR = TIM_EGR_UG;
}

void game_buzzer_tone(uint16_t frequency_hz) {
    uint32_t timer_hz = GAME_SYSTEM_CLOCK_HZ / 32u;
    uint32_t period;

    if (frequency_hz == 0u) {
        game_buzzer_off();
        return;
    }

    period = timer_hz / frequency_hz;
    if (period < 2u) {
        period = 2u;
    }

    BUZZER_TIM->ARR = period - 1u;
    /* 25% duty: half the drive time of the previous 50% square wave. */
    BUZZER_TIM->CCR3 = period / GAME_BUZZER_DUTY_DIVISOR;
    BUZZER_TIM->EGR = TIM_EGR_UG;
    BUZZER_TIM->CR1 |= TIM_CR1_CEN;
}

void game_buzzer_off(void) {
    BUZZER_TIM->CR1 &= ~TIM_CR1_CEN;
    BUZZER_TIM->CCR3 = 0u;
}

void game_system_reboot(void) {
    while (game_uart_tx_head != game_uart_tx_tail) {
    }
    while ((USART1->SR & USART_SR_TC) == 0u) {
    }
    NVIC_SystemReset();
}

void game_uart_putc(char character) {
    while (true) {
        uint32_t interrupt_state = __get_PRIMASK();
        uint8_t next;

        __disable_irq();
        next = (uint8_t)((game_uart_tx_head + 1u) %
                         GAME_UART_TX_BUFFER_SIZE);
        if (next != game_uart_tx_tail) {
            game_uart_tx_buffer[game_uart_tx_head] = (uint8_t)character;
            game_uart_tx_head = next;
            USART1->CR1 |= USART_CR1_TXEIE;
            if (interrupt_state == 0u) {
                __enable_irq();
            }
            return;
        }
        if (interrupt_state == 0u) {
            __enable_irq();
        }
        __NOP();
    }
}

void game_uart_puts(const char *string) {
    if (string == NULL) {
        return;
    }
    while (*string != '\0') {
        game_uart_putc(*string++);
    }
}

void game_uart_write(const uint8_t *data, size_t size) {
    if (data == NULL) {
        return;
    }
    for (size_t index = 0u; index < size; index++) {
        game_uart_putc((char)data[index]);
    }
}

bool game_uart_try_getc(char *character) {
    if ((character == NULL) || (game_uart_rx_head == game_uart_rx_tail)) {
        return false;
    }
    *character = (char)game_uart_rx_buffer[game_uart_rx_tail];
    game_uart_rx_tail =
        (uint8_t)((game_uart_rx_tail + 1u) % GAME_UART_RX_BUFFER_SIZE);
    return true;
}
