#include "bsp.h"

#include "system_stm32l1xx.h"

/* ============================================================================
 * Internal functions
 * ========================================================================= */

static uint32_t bsp_uart_calculate_brr(uint32_t peripheral_clock,
                                       uint32_t baudrate) {
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
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0u) {
    }

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR = (PWR->CR & ~PWR_CR_VOS) | PWR_CR_VOS_0;
    while ((PWR->CSR & PWR_CSR_VOSF) != 0u) {
    }

    FLASH->ACR = FLASH_ACR_ACC64 | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY;

    RCC->CFGR &=
        ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2 |
          RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL6 | RCC_CFGR_PLLDIV3;

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0u) {
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
    }

    SystemCoreClockUpdate();
}

/* ============================================================================
 * LED
 * ========================================================================= */

void bsp_led_init(void) {
    const uint32_t shift = BSP_LED_GPIO_PIN * 2u;

    RCC->AHBENR |= BSP_LED_GPIO_CLOCK;
    BSP_LED_GPIO_PORT->MODER &= ~(3u << shift);
    BSP_LED_GPIO_PORT->MODER |= 1u << shift;
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
    const uint32_t tx_shift = BSP_UART_TX_PIN * 2u;
    const uint32_t rx_shift = BSP_UART_RX_PIN * 2u;
    const uint32_t tx_af_shift = (BSP_UART_TX_PIN - 8u) * 4u;
    const uint32_t rx_af_shift = (BSP_UART_RX_PIN - 8u) * 4u;

    RCC->AHBENR |= BSP_UART_GPIO_CLOCK;
    RCC->APB2ENR |= BSP_UART_CLOCK;

    BSP_UART_GPIO_PORT->MODER &= ~((3u << tx_shift) | (3u << rx_shift));
    BSP_UART_GPIO_PORT->MODER |= (2u << tx_shift) | (2u << rx_shift);

    BSP_UART_GPIO_PORT->AFR[1] &=
        ~((0xFu << tx_af_shift) | (0xFu << rx_af_shift));
    BSP_UART_GPIO_PORT->AFR[1] |=
        (BSP_UART_AF << tx_af_shift) | (BSP_UART_AF << rx_af_shift);

    BSP_UART->CR1 = 0u;
    BSP_UART->CR2 = 0u;
    BSP_UART->CR3 = 0u;
    BSP_UART->BRR =
        bsp_uart_calculate_brr(BSP_SYS_CLOCK_HZ, BSP_UART_BAUDRATE);
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
