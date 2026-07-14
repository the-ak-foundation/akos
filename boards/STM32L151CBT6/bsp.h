#ifndef __BSP_H__
#define __BSP_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stm32l1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * BSP configuration define
 * ========================================================================= */

#define BSP_SYS_CLOCK_HZ (32000000)

/* ============================================================================
 * LED define
 * ========================================================================= */

/* LED: PB6, active high. */
#define BSP_LED_GPIO_PORT  GPIOB
#define BSP_LED_GPIO_PIN   (6)
#define BSP_LED_GPIO_CLOCK RCC_AHBENR_GPIOBEN
#define BSP_LED_ACTIVE_LOW (0)

/* ============================================================================
 * UART define
 * ========================================================================= */

#define BSP_UART_BAUDRATE   (115200)
#define BSP_UART            USART1
#define BSP_UART_GPIO_PORT  GPIOA
#define BSP_UART_TX_PIN     (9)
#define BSP_UART_RX_PIN     (10)
#define BSP_UART_GPIO_CLOCK RCC_AHBENR_GPIOAEN
#define BSP_UART_CLOCK      RCC_APB2ENR_USART1EN
#define BSP_UART_AF         (7)

/* ============================================================================
 * BSP initialization
 * ========================================================================= */

void bsp_init(void);

/* ============================================================================
 * Clock
 * ========================================================================= */

void bsp_clock_init(void);

/* ============================================================================
 * LED
 * ========================================================================= */

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
void bsp_uart_puts(const char* string);
void bsp_uart_write(const uint8_t* data, size_t size);
char bsp_uart_getc(void);
bool bsp_uart_try_getc(char* character);
bool bsp_uart_rx_ready(void);
bool bsp_uart_tx_ready(void);
void bsp_uart_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H__ */
