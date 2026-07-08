#include "console.h"

#include <stdint.h>

#include "akos.h"
#include "stm32l1xx.h"

#define CONSOLE_GPIO                 GPIOA
#define CONSOLE_TX_PIN               9u
#define CONSOLE_RX_PIN               10u
#define CONSOLE_USART_AF             7u
#define CONSOLE_BAUDRATE             115200u

static void console_gpio_init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    CONSOLE_GPIO->MODER &= ~((0x3u << (CONSOLE_TX_PIN * 2u)) |
                             (0x3u << (CONSOLE_RX_PIN * 2u)));
    CONSOLE_GPIO->MODER |= ((0x2u << (CONSOLE_TX_PIN * 2u)) |
                            (0x2u << (CONSOLE_RX_PIN * 2u)));

    CONSOLE_GPIO->OTYPER &= ~((1u << CONSOLE_TX_PIN) | (1u << CONSOLE_RX_PIN));

    CONSOLE_GPIO->OSPEEDR &= ~((0x3u << (CONSOLE_TX_PIN * 2u)) |
                               (0x3u << (CONSOLE_RX_PIN * 2u)));
    CONSOLE_GPIO->OSPEEDR |= ((0x2u << (CONSOLE_TX_PIN * 2u)) |
                              (0x2u << (CONSOLE_RX_PIN * 2u)));

    CONSOLE_GPIO->PUPDR &= ~((0x3u << (CONSOLE_TX_PIN * 2u)) |
                             (0x3u << (CONSOLE_RX_PIN * 2u)));
    CONSOLE_GPIO->PUPDR |= ((0x1u << (CONSOLE_TX_PIN * 2u)) |
                            (0x1u << (CONSOLE_RX_PIN * 2u)));

    CONSOLE_GPIO->AFR[1] &= ~((0xFu << ((CONSOLE_TX_PIN - 8u) * 4u)) |
                              (0xFu << ((CONSOLE_RX_PIN - 8u) * 4u)));
    CONSOLE_GPIO->AFR[1] |= ((CONSOLE_USART_AF << ((CONSOLE_TX_PIN - 8u) * 4u)) |
                             (CONSOLE_USART_AF << ((CONSOLE_RX_PIN - 8u) * 4u)));
}

void console_write_char(int ch)
{
    while ((USART1->SR & USART_SR_TXE) == 0u)
    {
    }

    USART1->DR = (uint16_t)(uint8_t)ch;
}

void console_init(void)
{
    console_gpio_init();

    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    USART1->CR1 = 0u;
    USART1->CR2 = 0u;
    USART1->CR3 = 0u;
    USART1->BRR = (uint16_t)((SystemCoreClock + (CONSOLE_BAUDRATE / 2u)) / CONSOLE_BAUDRATE);
    USART1->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;

    (void)USART1->SR;
    (void)USART1->DR;

    NVIC_SetPriority(USART1_IRQn, 2u);
    NVIC_EnableIRQ(USART1_IRQn);

    xdev_out(console_write_char);
}

void USART1_IRQHandler(void)
{
    if ((USART1->SR & USART_SR_RXNE) != 0u)
    {
        akos_shell_rx_isr((uint8_t)USART1->DR);
    }
}
