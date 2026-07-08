# AKOS 01-Blink-CM0

This is a minimal bare-metal AKOS blink example for a Cortex-M0 target.

What it shows:
- one statically registered AKOS thread
- direct register-level GPIO control
- Cortex-M0 port build setup
- reduced AKOS pool sizes for a small target

Files:
- `main.c` - AKOS entry point and blink thread
- `board.h` - board helper layer for GPIO and LED control
- `stm32f030xx.ld` - STM32F030 linker script
- `startup_stm32f030x6.s` - reset and vector setup

Default assumptions:
- STM32F030x6 target
- LED on `PA4`
- image linked at `0x08000000`

Build and flash:
- `cd examples/01-blink-cm0`
- `make`
- `make flash`

If your board is different, update `board.h`, the linker script, and the flash
address in the `Makefile`.
