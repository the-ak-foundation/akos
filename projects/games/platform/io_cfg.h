#ifndef __GAME_IO_CFG_H__
#define __GAME_IO_CFG_H__

#include <stdint.h>

#include "stm32l1xx.h"

/* Buttons, active low. */
#define BUTTON_DOWN_IO_PIN   (3u)
#define BUTTON_DOWN_IO_PORT  (GPIOB)
#define BUTTON_DOWN_IO_CLOCK (RCC_AHBENR_GPIOBEN)
#define BUTTON_UP_IO_PIN     (13u)
#define BUTTON_UP_IO_PORT    (GPIOC)
#define BUTTON_UP_IO_CLOCK   (RCC_AHBENR_GPIOCEN)
#define BUTTON_MODE_IO_PIN   (4u)
#define BUTTON_MODE_IO_PORT  (GPIOB)
#define BUTTON_MODE_IO_CLOCK (RCC_AHBENR_GPIOBEN)

/* Life LED. The two game status LEDs are PA2 and PA3. */
#define LED_LIFE_IO_PIN      (8u)
#define LED_LIFE_IO_PORT     (GPIOB)
#define LED_LIFE_IO_CLOCK    (RCC_AHBENR_GPIOBEN)
#define LED_STATUS_1_IO_PIN  (2u)
#define LED_STATUS_1_IO_PORT (GPIOA)
#define LED_STATUS_2_IO_PIN  (3u)
#define LED_STATUS_2_IO_PORT (GPIOA)

/* Buzzer: TIM3 channel 3 on PB0, alternate function 2. */
#define BUZZER_IO_PIN    (0u)
#define BUZZER_IO_PORT   (GPIOB)
#define BUZZER_IO_CLOCK  (RCC_AHBENR_GPIOBEN)
#define BUZZER_IO_AF     (2u)
#define BUZZER_TIM       (TIM3)
#define BUZZER_TIM_CLOCK (RCC_APB1ENR_TIM3EN)
#define BUZZER_TIM_IRQ   (TIM3_IRQn)

/* W25Q80 external flash chip select. SPI pins are configured by its driver. */
#define FLASH_CE_IO_PIN   (14u)
#define FLASH_CE_IO_PORT  (GPIOB)
#define FLASH_CE_IO_CLOCK (RCC_AHBENR_GPIOBEN)

/* Battery ADC input: ADC1 channel 0 on PA0. */
#define BAT_ADC_PIN      (0u)
#define BAT_ADC_PORT     (GPIOA)
#define BAT_ADC_CHANNEL  (0u)
#define BAT_ADC_IO_CLOCK (RCC_AHBENR_GPIOAEN)
#define BAT_ADC_CLOCK    (RCC_APB2ENR_ADC1EN)

/* SSD1309/SH1106 OLED software-I2C pins. */
#define OLED_CLK_PIN       (0x03u)
#define OLED_DATA_PIN      (0x04u)
#define OLED_RES_PIN       (0x08u)
#define OLED_CLK_IO_PIN    (13u)
#define OLED_CLK_IO_PORT   (GPIOB)
#define OLED_CLK_IO_CLOCK  (RCC_AHBENR_GPIOBEN)
#define OLED_DATA_IO_PIN   (12u)
#define OLED_DATA_IO_PORT  (GPIOB)
#define OLED_DATA_IO_CLOCK (RCC_AHBENR_GPIOBEN)
#define OLED_RES_IO_PIN    (15u)
#define OLED_RES_IO_PORT   (GPIOA)
#define OLED_RES_IO_CLOCK  (RCC_AHBENR_GPIOAEN)

/* USART1 console: TX PA9, RX PA10, alternate function 7. */
#define UART1_TX_IO_PIN   (9u)
#define UART1_RX_IO_PIN   (10u)
#define UART1_IO_PORT     (GPIOA)
#define UART1_IO_CLOCK    (RCC_AHBENR_GPIOAEN)
#define UART1_CLOCK       (RCC_APB2ENR_USART1EN)
#define UART1_AF          (7u)

#define EEPROM_BASE_ADDRESS (0x08080000u)
#define EEPROM_MAX_SIZE     (0x1000u)

void timer_delay_init(void);
void delay(uint32_t milliseconds);

void io_button_mode_init(void);
void io_button_up_init(void);
void io_button_down_init(void);
uint8_t io_button_mode_read(void);
uint8_t io_button_up_read(void);
uint8_t io_button_down_read(void);

void led_life_init(void);
void led_life_on(void);
void led_life_off(void);
void led_life_toggle(void);

void oled_clk_input_mode(void);
void oled_clk_output_mode(void);
void oled_clk_digital_write_low(void);
void oled_clk_digital_write_high(void);
int oled_clk_digital_read(void);

void oled_data_input_mode(void);
void oled_data_output_mode(void);
void oled_data_digital_write_low(void);
void oled_data_digital_write_high(void);
int oled_data_digital_read(void);

void oled_res_input_mode(void);
void oled_res_output_mode(void);
void oled_res_digital_write_low(void);
void oled_res_digital_write_high(void);
int oled_res_digital_read(void);

#endif /* __GAME_IO_CFG_H__ */
