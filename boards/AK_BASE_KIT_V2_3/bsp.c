#include "bsp.h"

#include "system_stm32l1xx.h"

/* ============================================================================
 * Internal functions
 * ========================================================================= */

static uint32_t bsp_uart_calculate_brr(uint32_t peripheral_clock, uint32_t baudrate) {
    return (peripheral_clock + (baudrate / 2u)) / baudrate;
}

static void bsp_led_pin_init(GPIO_TypeDef* gpio_port, uint32_t pin) {
    const uint32_t shift      = pin * 2u;
    const uint32_t field_mask = 3u << shift;
    const uint32_t pin_mask   = 1u << pin;

    gpio_port->MODER = (gpio_port->MODER & ~field_mask) | (1u << shift);
    gpio_port->OTYPER &= ~pin_mask;
    gpio_port->OSPEEDR &= ~field_mask;
    gpio_port->PUPDR &= ~field_mask;
    gpio_port->BSRR = 1u << (pin + 16u);
}

static void bsp_led_pin_toggle(GPIO_TypeDef* gpio_port, uint32_t pin) {
    const uint32_t pin_mask = 1u << pin;

    if ((gpio_port->ODR & pin_mask) != 0u) {
        gpio_port->BSRR = 1u << (pin + 16u);
    }
    else {
        gpio_port->BSRR = pin_mask;
    }
}

/* ============================================================================
 * BSP initialization
 * ========================================================================= */

void bsp_init(void) {
    /*
     * The AK Base Kit V2&3 bootloader leaves its one-shot application jump request
     * at 0x2000037C. Clear it explicitly so later resets enter the bootloader,
     * regardless of whether the application .bss range overlaps that address.
     */
    BSP_BOOT_JUMP_REQUEST = 0u;
    __DSB();

    bsp_led_init();
#if BSP_UART_ENABLED
    bsp_uart_init();
#endif
}

/* ============================================================================
 * LED
 * ========================================================================= */

void bsp_led_init(void) {
    RCC->AHBENR |= BSP_LED_GPIO_CLOCK | BSP_UART_GPIO_CLOCK;

    bsp_led_pin_init(GPIOB, BSP_LED_GPIO_PIN);
    bsp_led_pin_init(GPIOA, BSP_LED_PA2_GPIO_PIN);
    bsp_led_pin_init(GPIOA, BSP_LED_PA3_GPIO_PIN);
}

void bsp_led_on(void) {
    BSP_LED_GPIO_PORT->BSRR = 1u << BSP_LED_GPIO_PIN;
    GPIOA->BSRR             = (1u << BSP_LED_PA2_GPIO_PIN) | (1u << BSP_LED_PA3_GPIO_PIN);
}

void bsp_led_off(void) {
    BSP_LED_GPIO_PORT->BSRR = 1u << (BSP_LED_GPIO_PIN + 16u);
    GPIOA->BSRR = (1u << (BSP_LED_PA2_GPIO_PIN + 16u)) |
                  (1u << (BSP_LED_PA3_GPIO_PIN + 16u));
}

void bsp_led_toggle(void) {
    bsp_led_pin_toggle(BSP_LED_GPIO_PORT, BSP_LED_GPIO_PIN);
    bsp_led_pin_toggle(GPIOA, BSP_LED_PA2_GPIO_PIN);
    bsp_led_pin_toggle(GPIOA, BSP_LED_PA3_GPIO_PIN);
}

void bsp_led_write(bool state) {
    if (state) {
        bsp_led_on();
    }
    else {
        bsp_led_off();
    }
}

bool bsp_led_is_on(void) {
    const bool pin_is_high = (BSP_LED_GPIO_PORT->ODR & (1u << BSP_LED_GPIO_PIN)) != 0u;

#if BSP_LED_ACTIVE_LOW
    return !pin_is_high;
#else
    return pin_is_high;
#endif
}

/* ============================================================================
 * UART
 * ========================================================================= */

void bsp_uart_init(void) {
    const uint32_t tx_shift    = BSP_UART_TX_PIN * 2u;
    const uint32_t rx_shift    = BSP_UART_RX_PIN * 2u;
    const uint32_t tx_af_shift = (BSP_UART_TX_PIN - 8u) * 4u;
    const uint32_t rx_af_shift = (BSP_UART_RX_PIN - 8u) * 4u;
    const uint32_t tx_pin_mask = 1u << BSP_UART_TX_PIN;
    const uint32_t rx_pin_mask = 1u << BSP_UART_RX_PIN;
    const uint32_t mode_mask   = (3u << tx_shift) | (3u << rx_shift);

    RCC->AHBENR |= BSP_UART_GPIO_CLOCK;
    RCC->APB2ENR |= BSP_UART_CLOCK;

    BSP_UART_GPIO_PORT->MODER &= ~mode_mask;
    BSP_UART_GPIO_PORT->MODER |= (2u << tx_shift) | (2u << rx_shift);
    BSP_UART_GPIO_PORT->OTYPER &= ~(tx_pin_mask | rx_pin_mask);
    BSP_UART_GPIO_PORT->OSPEEDR &= ~mode_mask;
    BSP_UART_GPIO_PORT->PUPDR &= ~mode_mask;
    BSP_UART_GPIO_PORT->PUPDR |= (1u << tx_shift) | (1u << rx_shift);

    BSP_UART_GPIO_PORT->AFR[1] &= ~((0xFu << tx_af_shift) | (0xFu << rx_af_shift));
    BSP_UART_GPIO_PORT->AFR[1] |= (BSP_UART_AF << tx_af_shift) | (BSP_UART_AF << rx_af_shift);

    BSP_UART->CR1 = 0u;
    BSP_UART->CR2 = 0u;
    BSP_UART->CR3 = 0u;
    BSP_UART->BRR = bsp_uart_calculate_brr(SystemCoreClock, BSP_UART_BAUDRATE);
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
        bsp_uart_putc(*string++);
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
    if ((character == NULL) || !bsp_uart_rx_ready()) {
        return false;
    }
    *character = (char)(BSP_UART->DR & 0xFFu);
    return true;
}

void bsp_uart_flush(void) {
    while ((BSP_UART->SR & USART_SR_TC) == 0u) {
    }
}
