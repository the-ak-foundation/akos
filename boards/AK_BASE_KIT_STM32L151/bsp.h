#ifndef __AK_BASE_KIT_STM32L151_BSP_H__
#define __AK_BASE_KIT_STM32L151_BSP_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stm32l1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Peripheral base addresses
 * ========================================================================= */

#ifndef BSP_RCC_BASE
#define BSP_RCC_BASE    (0x40023800u)
#endif

#ifndef BSP_GPIOA_BASE
#define BSP_GPIOA_BASE  (0x40020000u)
#endif

#ifndef BSP_GPIOB_BASE
#define BSP_GPIOB_BASE  (0x40020400u)
#endif

#ifndef BSP_USART1_BASE
#define BSP_USART1_BASE (0x40013800u)
#endif

/* ============================================================================
 * Register access
 * ========================================================================= */

#define BSP_REG32(address) (*(volatile uint32_t *)(uintptr_t)(address))

/* RCC registers. */
#define BSP_RCC_AHBENR_OFFSET  (0x1Cu)
#define BSP_RCC_APB2ENR_OFFSET (0x20u)

#define BSP_RCC_AHBENR  BSP_REG32(BSP_RCC_BASE + BSP_RCC_AHBENR_OFFSET)
#define BSP_RCC_APB2ENR BSP_REG32(BSP_RCC_BASE + BSP_RCC_APB2ENR_OFFSET)

/* GPIO registers. */
#define BSP_GPIO_MODER_OFFSET   (0x00u)
#define BSP_GPIO_OTYPER_OFFSET  (0x04u)
#define BSP_GPIO_OSPEEDR_OFFSET (0x08u)
#define BSP_GPIO_PUPDR_OFFSET   (0x0Cu)
#define BSP_GPIO_ODR_OFFSET     (0x14u)
#define BSP_GPIO_BSRR_OFFSET    (0x18u)
#define BSP_GPIO_AFRL_OFFSET    (0x20u)
#define BSP_GPIO_AFRH_OFFSET    (0x24u)

#define BSP_GPIO_MODER(base)   BSP_REG32((base) + BSP_GPIO_MODER_OFFSET)
#define BSP_GPIO_OTYPER(base)  BSP_REG32((base) + BSP_GPIO_OTYPER_OFFSET)
#define BSP_GPIO_OSPEEDR(base) BSP_REG32((base) + BSP_GPIO_OSPEEDR_OFFSET)
#define BSP_GPIO_PUPDR(base)   BSP_REG32((base) + BSP_GPIO_PUPDR_OFFSET)
#define BSP_GPIO_ODR(base)     BSP_REG32((base) + BSP_GPIO_ODR_OFFSET)
#define BSP_GPIO_BSRR(base)    BSP_REG32((base) + BSP_GPIO_BSRR_OFFSET)
#define BSP_GPIO_AFRL(base)    BSP_REG32((base) + BSP_GPIO_AFRL_OFFSET)
#define BSP_GPIO_AFRH(base)    BSP_REG32((base) + BSP_GPIO_AFRH_OFFSET)

/* USART registers. */
#define BSP_USART_SR_OFFSET  (0x00u)
#define BSP_USART_DR_OFFSET  (0x04u)
#define BSP_USART_BRR_OFFSET (0x08u)
#define BSP_USART_CR1_OFFSET (0x0Cu)
#define BSP_USART_CR2_OFFSET (0x10u)
#define BSP_USART_CR3_OFFSET (0x14u)

#define BSP_USART_SR(base)  BSP_REG32((base) + BSP_USART_SR_OFFSET)
#define BSP_USART_DR(base)  BSP_REG32((base) + BSP_USART_DR_OFFSET)
#define BSP_USART_BRR(base) BSP_REG32((base) + BSP_USART_BRR_OFFSET)
#define BSP_USART_CR1(base) BSP_REG32((base) + BSP_USART_CR1_OFFSET)
#define BSP_USART_CR2(base) BSP_REG32((base) + BSP_USART_CR2_OFFSET)
#define BSP_USART_CR3(base) BSP_REG32((base) + BSP_USART_CR3_OFFSET)

/* AK Base Kit bootloader handoff state in retained SRAM. */
#define BSP_BOOT_JUMP_REQUEST_ADDRESS (0x2000037Cu)
#define BSP_BOOT_JUMP_REQUEST         BSP_REG32(BSP_BOOT_JUMP_REQUEST_ADDRESS)

/* ============================================================================
 * BSP configuration
 * ========================================================================= */

#define BSP_SYS_CLOCK_HZ (32000000u)
#define BSP_UART_ENABLED  (1u)

/* Board LEDs: life PB8, status PA2, and status PA3; all active high. */
#define BSP_LED_GPIO_BASE    BSP_GPIOB_BASE
#define BSP_LED_GPIO_PIN     (8u)
#define BSP_LED_GPIO_CLOCK   (1u << 1)
#define BSP_LED_PA2_GPIO_PIN (2u)
#define BSP_LED_PA3_GPIO_PIN (3u)
#define BSP_LED_ACTIVE_LOW   (0u)

/* Console: USART1, TX PA9, RX PA10, alternate function 7. */
#define BSP_UART_BASE       BSP_USART1_BASE
#define BSP_UART_BAUDRATE   (115200u)
#define BSP_UART_GPIO_BASE  BSP_GPIOA_BASE
#define BSP_UART_TX_PIN     (9u)
#define BSP_UART_RX_PIN     (10u)
#define BSP_UART_GPIO_CLOCK (1u << 0)
#define BSP_UART_CLOCK      (1u << 14)
#define BSP_UART_AF         (7u)

/* Temporary CMSIS aliases retained until bsp.c uses the register accessors. */
#define BSP_LED_GPIO_PORT  GPIOB
#define BSP_UART           USART1
#define BSP_UART_GPIO_PORT GPIOA

/* ============================================================================
 * BSP initialization
 * ========================================================================= */

void bsp_init(void);

/* ============================================================================
 * LED
 * ========================================================================= */

/* These APIs control PB8, PA2, and PA3 together. */
void bsp_led_init(void);
void bsp_led_on(void);
void bsp_led_off(void);
void bsp_led_toggle(void);
void bsp_led_write(bool state);
bool bsp_led_is_on(void);

/* ============================================================================
 * UART
 * ========================================================================= */

void bsp_uart_init(void);
void bsp_uart_putc(char character);
void bsp_uart_puts(const char *string);
void bsp_uart_write(const uint8_t *data, size_t size);
char bsp_uart_getc(void);
bool bsp_uart_try_getc(char *character);
bool bsp_uart_rx_ready(void);
bool bsp_uart_tx_ready(void);
void bsp_uart_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* __AK_BASE_KIT_STM32L151_BSP_H__ */
