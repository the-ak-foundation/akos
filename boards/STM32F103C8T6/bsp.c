#include "bsp.h"

/* ============================================================================
 * Internal functions
 * ========================================================================= */

static void bsp_led_pin_init(GPIO_TypeDef* gpio_port, uint32_t gpio_pin) {
    const uint32_t pin_position          = (gpio_pin % 8u) * 4u;
    const uint32_t pin_mask              = 0x0Fu << pin_position;
    const uint32_t output_push_pull_2mhz = 0x02u << pin_position;
    volatile uint32_t* configuration_register;

    if (gpio_pin < 8u) {
        configuration_register = &gpio_port->CRL;
    } else {
        configuration_register = &gpio_port->CRH;
    }

    *configuration_register =
        (*configuration_register & ~pin_mask) | output_push_pull_2mhz;
}

static void bsp_led_pin_write(GPIO_TypeDef* gpio_port,
                              uint32_t gpio_pin,
                              bool active_low,
                              bool state) {
    const bool pin_is_high = state != active_low;

    if (pin_is_high) {
        gpio_port->BSRR = 1u << gpio_pin;
    } else {
        gpio_port->BSRR = 1u << (gpio_pin + 16u);
    }
}

static bool bsp_led_pin_is_on(GPIO_TypeDef* gpio_port,
                              uint32_t gpio_pin,
                              bool active_low) {
    const bool pin_is_high = (gpio_port->ODR & (1u << gpio_pin)) != 0u;

    return pin_is_high != active_low;
}

static uint32_t bsp_uart_calculate_brr(uint32_t peripheral_clock, uint32_t baudrate) {
    /*
     * STM32F1 USART uses oversampling by 16.
     *
     * BRR register representation:
     *
     * BRR[15:4] = USARTDIV mantissa
     * BRR[3:0]  = USARTDIV fraction
     *
     * For oversampling by 16:
     *
     * BRR = peripheral_clock / baudrate
     *
     * Add baudrate / 2 for integer rounding.
     */
    return (peripheral_clock + (baudrate / 2u)) / baudrate;
}

/* ============================================================================
 * BSP initialization
 * ========================================================================= */

void bsp_init(void) {
    bsp_led_init();
#if BSP_UART_ENABLED
    bsp_uart_init();
#endif
}

/* ============================================================================
 * LED
 * ========================================================================= */

void bsp_led_init(void) {
    /*
     * Enable the LED GPIO peripheral clock.
     */
    RCC->APB2ENR |= BSP_LED_PC13_GPIO_CLOCK | BSP_LED_PB2_GPIO_CLOCK;

    /*
     * GPIO configuration:
     *
     * MODE = 10: output mode, maximum speed 2 MHz
     * CNF  = 00: general-purpose push-pull output
     */
    bsp_led_pin_init(BSP_LED_PC13_GPIO_PORT, BSP_LED_PC13_GPIO_PIN);
    bsp_led_pin_init(BSP_LED_PB2_GPIO_PORT, BSP_LED_PB2_GPIO_PIN);

    /*
     * Default LED state: OFF.
     */
    bsp_led_off();
}

void bsp_led_on(void) {
    bsp_led_pin_write(BSP_LED_PC13_GPIO_PORT,
                      BSP_LED_PC13_GPIO_PIN,
                      BSP_LED_PC13_ACTIVE_LOW,
                      true);
    bsp_led_pin_write(BSP_LED_PB2_GPIO_PORT,
                      BSP_LED_PB2_GPIO_PIN,
                      BSP_LED_PB2_ACTIVE_LOW,
                      true);
}

void bsp_led_off(void) {
    bsp_led_pin_write(BSP_LED_PC13_GPIO_PORT,
                      BSP_LED_PC13_GPIO_PIN,
                      BSP_LED_PC13_ACTIVE_LOW,
                      false);
    bsp_led_pin_write(BSP_LED_PB2_GPIO_PORT,
                      BSP_LED_PB2_GPIO_PIN,
                      BSP_LED_PB2_ACTIVE_LOW,
                      false);
}

void bsp_led_toggle(void) {
    bsp_led_write(!bsp_led_is_on());
}

void bsp_led_write(bool state) {
    if (state) {
        bsp_led_on();
    } else {
        bsp_led_off();
    }
}

bool bsp_led_is_on(void) {
    return bsp_led_pin_is_on(BSP_LED_PB2_GPIO_PORT,
                             BSP_LED_PB2_GPIO_PIN,
                             BSP_LED_PB2_ACTIVE_LOW);
}

/* ============================================================================
 * UART
 * ========================================================================= */

void bsp_uart_init(void) {
    /*
     * USART1 pin mapping:
     *
     * PA9  -> USART1_TX
     * PA10 -> USART1_RX
     *
     * USART configuration:
     *
     * Baud rate : 115200
     * Data bits : 8
     * Parity    : none
     * Stop bits : 1
     * Flow ctrl : none
     */

    /*
     * Enable clocks:
     *
     * AFIO
     * GPIOA
     * USART1
     */
    RCC->APB2ENR |=
        BSP_UART_AFIO_CLOCK | BSP_UART_GPIO_CLOCK | BSP_UART_CLOCK;

    /*
     * PA9: alternate-function push-pull output, 50 MHz.
     *
     * MODE9 = 11
     * CNF9  = 10
     *
     * Configuration value = 0b1011 = 0xB
     */
    const uint32_t tx_position = (BSP_UART_TX_PIN - 8u) * 4u;

    BSP_UART_GPIO_PORT->CRH =
        (BSP_UART_GPIO_PORT->CRH & ~(0x0Fu << tx_position)) |
        (0x0Bu << tx_position);

    /*
     * PA10: floating input.
     *
     * MODE10 = 00
     * CNF10  = 01
     *
     * Configuration value = 0b0100 = 0x4
     */
    const uint32_t rx_position = (BSP_UART_RX_PIN - 8u) * 4u;

    BSP_UART_GPIO_PORT->CRH =
        (BSP_UART_GPIO_PORT->CRH & ~(0x0Fu << rx_position)) |
        (0x04u << rx_position);

    /*
     * Disable USART before configuration.
     */
    BSP_UART->CR1 = 0u;
    BSP_UART->CR2 = 0u;
    BSP_UART->CR3 = 0u;

    /*
     * USART1 is connected to APB2.
     *
     * USART1 uses PCLK2, which is configured to match the 72 MHz system clock.
     * Calculate BRR from SystemCoreClock so UART remains correct if the clock
     * configuration changes later.
     */
    BSP_UART->BRR = bsp_uart_calculate_brr(SystemCoreClock, BSP_UART_BAUDRATE);

    /*
     * CR1 configuration:
     *
     * M    = 0: 8 data bits
     * PCE  = 0: parity disabled
     * TE   = 1: transmitter enabled
     * RE   = 1: receiver enabled
     * UE   = 1: USART enabled
     */
    BSP_UART->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

bool bsp_uart_tx_ready(void) {
    return (BSP_UART->SR & USART_SR_TXE) != 0u;
}

bool bsp_uart_rx_ready(void) {
    return (BSP_UART->SR & USART_SR_RXNE) != 0u;
}

void bsp_uart_putc(char character) {
    while (!bsp_uart_tx_ready()) {
    }

    BSP_UART->DR = (uint8_t)character;
}

void bsp_uart_puts(const char* string) {
    if (string == NULL) {
        return;
    }

    while (*string != '\0') {
        bsp_uart_putc(*string);
        string++;
    }
}

void bsp_uart_write(const uint8_t* data, size_t size) {
    if (data == NULL) {
        return;
    }

    for (size_t index = 0u; index < size; index++) {
        bsp_uart_putc((char)data[index]);
    }
}

char bsp_uart_getc(void) {
    while (!bsp_uart_rx_ready()) {
    }

    return (char)(BSP_UART->DR & 0xFFu);
}

bool bsp_uart_try_getc(char* character) {
    if (character == NULL) {
        return false;
    }

    if (!bsp_uart_rx_ready()) {
        return false;
    }

    *character = (char)(BSP_UART->DR & 0xFFu);

    return true;
}

void bsp_uart_flush(void) {
    /*
     * Wait until:
     *
     * TXE = transmit data register empty
     * TC  = complete frame transmitted
     */
    while ((BSP_UART->SR & USART_SR_TC) == 0u) {
    }
}
