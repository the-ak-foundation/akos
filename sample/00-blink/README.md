# AKOS 00-Blink

This is a minimal bare-metal AKOS starter for three blinking LEDs.

What it gives you:
- three statically registered AKOS threads
- direct register-level GPIO control
- no HAL, no vendor framework, no dynamic allocation

Files:
- `main.c` - AKOS entry point and three blink threads
- `board.h` - tiny board helper layer for GPIO and LED control

Default assumptions:
- STM32L1-style GPIO register layout
- LEDs on `PB8`, `PA2`, and `PA3`
- GPIOA/GPIOB clock gates live in `RCC->AHBENR`

If your board is different, change the macros in `board.h`:
- `BLINK_GPIOA_BASE`
- `BLINK_GPIOB_BASE`
- `BLINK_LED_PB8_PIN`
- `BLINK_LED_PA2_PIN`
- `BLINK_LED_PA3_PIN`
- `BLINK_RCC_AHBENR_GPIOA_EN`
- `BLINK_RCC_AHBENR_GPIOB_EN`

Build note:
- Run `make` from `sample/00-blink` to build the sample image.
- The sample uses the STM32L1 vendor files in this folder: `stm32l1xx.h`, `system_stm32l1xx.c`, `system_stm32l1xx.h`, `startup_stm32l151xb.s`, and `stm32l151xx.ld`.
- Adjust the clock setup inside `system_stm32l1xx.c` for your board if needed.
- The sample uses AKOS kernel APIs, so it runs on the bare-metal target image once wired into your board support package.
- Finally run `make flash` from `sample/00-blink` to see PB8, PA2, PA3 blinks independently.
