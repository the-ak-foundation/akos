/**
 ******************************************************************************
 * @file    uart.h
 * @brief   USART1 console driver API
 *
 * @module  Game Hub - UART
 ******************************************************************************
 */

#ifndef __UART_H__
#define __UART_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void uart_init(void);
void uart_putc(char character);
void uart_puts(const char *string);
void uart_write(const uint8_t *data, size_t size);
bool uart_try_getc(char *character);
void uart_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H__ */
