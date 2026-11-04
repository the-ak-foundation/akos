/**
 ******************************************************************************
 * @file    uart.c
 * @brief   Interrupt-driven USART1 console driver
 *
 * @module  Game Hub - UART
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "uart.h"

#include "io_cfg.h"
#include "system_stm32l1xx.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
#define UART_BAUDRATE       (115200u)
#define UART_RX_BUFFER_SIZE (64u)
#define UART_TX_BUFFER_SIZE (128u)

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/
static volatile uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint8_t uart_rx_head;
static volatile uint8_t uart_rx_tail;
static volatile uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];
static volatile uint8_t uart_tx_head;
static volatile uint8_t uart_tx_tail;

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void uart_init(void) {
    uint32_t tx_shift = UART1_TX_IO_PIN * 2u;
    uint32_t rx_shift = UART1_RX_IO_PIN * 2u;
    uint32_t tx_af_shift = (UART1_TX_IO_PIN - 8u) * 4u;
    uint32_t rx_af_shift = (UART1_RX_IO_PIN - 8u) * 4u;
    uint32_t mode_mask = (3u << tx_shift) | (3u << rx_shift);

    RCC->AHBENR |= UART1_IO_CLOCK;
    RCC->APB2ENR |= UART1_CLOCK;

    UART1_IO_PORT->MODER =
        (UART1_IO_PORT->MODER & ~mode_mask) | (2u << tx_shift) | (2u << rx_shift);
    UART1_IO_PORT->OTYPER &= ~((1u << UART1_TX_IO_PIN) | (1u << UART1_RX_IO_PIN));
    UART1_IO_PORT->PUPDR =
        (UART1_IO_PORT->PUPDR & ~mode_mask) | (1u << tx_shift) | (1u << rx_shift);
    UART1_IO_PORT->AFR[1] =
        (UART1_IO_PORT->AFR[1] & ~((0xFu << tx_af_shift) | (0xFu << rx_af_shift))) |
        (UART1_AF << tx_af_shift) | (UART1_AF << rx_af_shift);

    USART1->CR1 = 0u;
    USART1->CR2 = 0u;
    USART1->CR3 = 0u;
    USART1->BRR = (SystemCoreClock + UART_BAUDRATE / 2u) / UART_BAUDRATE;
    uart_rx_head = 0u;
    uart_rx_tail = 0u;
    uart_tx_head = 0u;
    uart_tx_tail = 0u;
    NVIC_SetPriority(USART1_IRQn, 5u);
    NVIC_EnableIRQ(USART1_IRQn);
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;
}

void USART1_IRQHandler(void) {
    uint32_t status = USART1->SR;

    if ((status & (USART_SR_RXNE | USART_SR_ORE)) != 0u) {
        uint8_t data = (uint8_t)USART1->DR;

        if ((status & USART_SR_RXNE) != 0u) {
            uint8_t next = (uint8_t)((uart_rx_head + 1u) % UART_RX_BUFFER_SIZE);

            if (next != uart_rx_tail) {
                uart_rx_buffer[uart_rx_head] = data;
                uart_rx_head = next;
            }
        }
    }

    if (((status & USART_SR_TXE) != 0u) && ((USART1->CR1 & USART_CR1_TXEIE) != 0u)) {
        if (uart_tx_tail != uart_tx_head) {
            USART1->DR = uart_tx_buffer[uart_tx_tail];
            uart_tx_tail = (uint8_t)((uart_tx_tail + 1u) % UART_TX_BUFFER_SIZE);
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}

void uart_putc(char character) {
    while (true) {
        uint32_t interrupt_state = __get_PRIMASK();
        uint8_t next;

        __disable_irq();
        next = (uint8_t)((uart_tx_head + 1u) % UART_TX_BUFFER_SIZE);
        if (next != uart_tx_tail) {
            uart_tx_buffer[uart_tx_head] = (uint8_t)character;
            uart_tx_head = next;
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

void uart_puts(const char *string) {
    if (string == NULL) {
        return;
    }
    while (*string != '\0') {
        uart_putc(*string++);
    }
}

void uart_write(const uint8_t *data, size_t size) {
    if (data == NULL) {
        return;
    }
    for (size_t index = 0u; index < size; index++) {
        uart_putc((char)data[index]);
    }
}

bool uart_try_getc(char *character) {
    if ((character == NULL) || (uart_rx_head == uart_rx_tail)) {
        return false;
    }
    *character = (char)uart_rx_buffer[uart_rx_tail];
    uart_rx_tail = (uint8_t)((uart_rx_tail + 1u) % UART_RX_BUFFER_SIZE);
    return true;
}

void uart_flush(void) {
    while (uart_tx_head != uart_tx_tail) {
    }
    while ((USART1->SR & USART_SR_TC) == 0u) {
    }
}
