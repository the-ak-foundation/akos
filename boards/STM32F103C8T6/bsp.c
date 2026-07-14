#include "bsp.h"

/* ============================================================================
 * Internal functions
 * ========================================================================= */

static uint32_t bsp_uart_calculate_brr(uint32_t peripheral_clock,
                                       uint32_t baudrate) {
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
    bsp_clock_init();
    bsp_led_init();
    bsp_uart_init();
}

/* ============================================================================
 * Clock
 * ========================================================================= */

void bsp_clock_init(void) {
    /*
     * Clock tree:
     *
     * HSE      = 8 MHz
     * PLL      = HSE x 9 = 72 MHz
     *
     * SYSCLK   = 72 MHz
     * HCLK     = 72 MHz
     * PCLK1    = 36 MHz
     * PCLK2    = 72 MHz
     */

    /*
     * Enable HSI first so the MCU always has a valid clock source while
     * changing the clock configuration.
     */
    RCC->CR |= RCC_CR_HSION;

    while ((RCC->CR & RCC_CR_HSIRDY) == 0u) {
    }

    /*
     * Switch temporarily to HSI before changing PLL configuration.
     */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {
    }

    /*
     * Disable PLL before modifying PLL configuration.
     */
    RCC->CR &= ~RCC_CR_PLLON;

    while ((RCC->CR & RCC_CR_PLLRDY) != 0u) {
    }

    /*
     * Enable external 8 MHz crystal oscillator.
     */
    RCC->CR |= RCC_CR_HSEON;

    while ((RCC->CR & RCC_CR_HSERDY) == 0u) {
    }

    /*
     * Configure Flash:
     *
     * 72 MHz requires two wait states.
     * Enable Flash prefetch buffer.
     */
    FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);
    FLASH->ACR |= FLASH_ACR_LATENCY_2 | FLASH_ACR_PRFTBE;

    /*
     * Clear clock configuration fields.
     */
    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE1 |
                   RCC_CFGR_PPRE2 | RCC_CFGR_ADCPRE | RCC_CFGR_PLLSRC |
                   RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);

    /*
     * Configure clock prescalers:
     *
     * AHB  = SYSCLK / 1 = 72 MHz
     * APB1 = HCLK / 2   = 36 MHz
     * APB2 = HCLK / 1   = 72 MHz
     *
     * PLL source = HSE
     * PLL input  = HSE without division
     * PLL MUL    = x9
     */
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV2 |
                 RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;

    /*
     * Enable PLL.
     */
    RCC->CR |= RCC_CR_PLLON;

    while ((RCC->CR & RCC_CR_PLLRDY) == 0u) {
    }

    /*
     * Select PLL as system clock.
     */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
    }

    /*
     * Update CMSIS SystemCoreClock variable.
     *
     * The project should define:
     *
     * HSE_VALUE = 8000000
     */
    SystemCoreClockUpdate();
}

/* ============================================================================
 * LED
 * ========================================================================= */

void bsp_led_init(void) {
    /*
     * Enable GPIOC peripheral clock.
     */
    RCC->APB2ENR |= BSP_LED_GPIO_CLOCK;

    /*
     * PC13 is located in GPIOC_CRH.
     *
     * GPIO configuration:
     *
     * MODE13 = 10: output mode, maximum speed 2 MHz
     * CNF13  = 00: general-purpose push-pull output
     *
     * CRH bits for PC13:
     *
     * Bits 23:20
     */
    const uint32_t pin_position          = (BSP_LED_GPIO_PIN - 8u) * 4u;
    const uint32_t pin_mask              = 0x0Fu << pin_position;
    const uint32_t output_push_pull_2mhz = 0x02u << pin_position;

    BSP_LED_GPIO_PORT->CRH =
        (BSP_LED_GPIO_PORT->CRH & ~pin_mask) | output_push_pull_2mhz;

    /*
     * Default LED state: OFF.
     */
    bsp_led_off();
}

void bsp_led_on(void) {
#if BSP_LED_ACTIVE_LOW
    BSP_LED_GPIO_PORT->BSRR = 1u << (BSP_LED_GPIO_PIN + 16u);
#else
    BSP_LED_GPIO_PORT->BSRR = 1u << BSP_LED_GPIO_PIN;
#endif
}

void bsp_led_off(void) {
#if BSP_LED_ACTIVE_LOW
    BSP_LED_GPIO_PORT->BSRR = 1u << BSP_LED_GPIO_PIN;
#else
    BSP_LED_GPIO_PORT->BSRR = 1u << (BSP_LED_GPIO_PIN + 16u);
#endif
}

void bsp_led_toggle(void) {
    const uint32_t pin_mask = 1u << BSP_LED_GPIO_PIN;

    if ((BSP_LED_GPIO_PORT->ODR & pin_mask) != 0u) {
        BSP_LED_GPIO_PORT->BSRR = 1u << (BSP_LED_GPIO_PIN + 16u);
    } else {
        BSP_LED_GPIO_PORT->BSRR = 1u << BSP_LED_GPIO_PIN;
    }
}

void bsp_led_write(bool state) {
    if (state) {
        bsp_led_on();
    } else {
        bsp_led_off();
    }
}

bool bsp_led_is_on(void) {
    const bool pin_is_high =
        (BSP_LED_GPIO_PORT->ODR & (1u << BSP_LED_GPIO_PIN)) != 0u;

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
     * PCLK2 = 72 MHz
     *
     * BRR for 115200 baud:
     *
     * 72,000,000 / 115,200 = 625
     * BRR = 0x0271
     */
    BSP_UART->BRR = bsp_uart_calculate_brr(BSP_SYS_CLOCK_HZ, BSP_UART_BAUDRATE);

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
