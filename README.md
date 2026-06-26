# AKOS - Real-Time Operating System

[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)
[![Static Badge](https://img.shields.io/badge/AKOS-User_Manual-red)](https://the-ak-foundation.github.io/akos-docs)

AKOS is an open-source preemptive real-time operating system designed for learning and reference purposes in embedded systems education. It combines a priority-based scheduler with an event-driven programming model built around tasks, signals, timers, and state machines.

AKOS helps students understand how an RTOS works internally and serves as a practical reference for those who want to build their own real-time operating system from the ground up.

## Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│ APPLICATION LAYERS                                                  │
│                                                                     │
│ Tasks · Signals · Software Timers · Logic                           │
└─────────────────────────────────────────────────────────────────────┘
┌─────────────────────────────────┐ ┌─────────────────────────────────┐
│ AKOS RTOS                       │ │ PLATFORM SUPPORT LAYER          │
│                                 │ │                                 │
│ ┌─────────────────────────────┐ │ │ ┌─────────────────────────────┐ │
│ │ KERNEL SERVICES             │ │ │ │ BSP                         │ │
│ │                             │ │ │ │                             │ │
│ │ Core · Thread               │ │ │ │ Startup · Vector Table      │ │      
│ │ Memory · Message            │ │ │ │ Clock · Pinmux · Console    │ │
│ └─────────────────────────────┘ │ │ └─────────────────────────────┘ │
│                                 │ │                                 │
│ ┌─────────────────────────────┐ │ │ ┌─────────────────────────────┐ │
│ │ PORT                        │ │ │ │ DRIVERS                     │ │
│ │                             │ │ │ │                             │ │
│ │ Context Switch · SysTick    │ │ │ │ GPIO · UART · Timer         │ │
│ │ SVC · PendSV · IRQ Ctrl     │ │ │ │ I2C · SPI · Flash           │ │
│ │ Stack Frame                 │ │ │ │                             │ │
│ └─────────────────────────────┘ │ │ └─────────────────────────────┘ │
└─────────────────────────────────┘ └─────────────────────────────────┘
┌─────────────────────────────────────────────────────────────────────┐
│ HARDWARE LAYER                                                      │
│                                                                     │
│ CPU Core · NVIC · SysTick · Flash · SRAM                            │
│ GPIO · UART · Timer · I2C · SPI · External Devices                  │
└─────────────────────────────────────────────────────────────────────┘
```
## Hardware support

| Board | MCU | Status |
|-------|-----|--------|
| [AK Embedded Base Kit](https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu) | STM32L151CBT6 (ARM Cortex-M3) | Primary target |
| STM32F030F4P6 board | STM32F030F4P6 (ARM Cortex-M0) | Example target |
| TI LP-MSPM0C1104 LaunchPad | MSPM0C1104 (ARM Cortex-M0+) | Example target |

## Quick start

**Prerequisites:**
```bash
sudo apt-get install gcc-arm-none-eabi cmake ninja-build
```

**The blink LED example on [AK Embedded Base Kit](https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu):**

[<img src="https://the-ak-foundation.github.io/akos-docs/02_result.gif" width="480"/>](https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu)

```bash
git clone https://github.com/the-ak-foundation/akos
cd akos
make -C examples/ARM/cortex-m3/STM32L151CBT6 help
make -C examples/ARM/cortex-m3/STM32L151CBT6 clean all
make -C examples/ARM/cortex-m3/STM32L151CBT6 flash
```

## Examples

The best starting point is [`examples/README.md`](examples/README.md).

The current example layout includes:

- [`examples/ARM/cortex-m3/STM32L151CBT6`](examples/ARM/cortex-m3/STM32L151CBT6) for the STM32L151 Cortex-M3 target
- [`examples/ARM/cortex-m0/STM32F030F4P6`](examples/ARM/cortex-m0/STM32F030F4P6) for STM32F030 Cortex-M0 examples
- [`examples/ARM/cortex-m0/LP-MSPM0C1104`](examples/ARM/cortex-m0/LP-MSPM0C1104) for the TI MSPM0C1104 Cortex-M0+ example
- Static thread registration with `AKOS_THREAD_DEFINE(...)`
- Board `Makefile` files that include `kernel/kernel.mk`
- Board linker scripts that include `kernel.ld` between flash start and flash end
- Example configuration guidance in `kernel/config.mk` and `kernel/inc/config.h`

If you are bringing up a new board, start from the closest board folder under `examples/ARM`, then adapt the platform files, linker script, startup file, and kernel config settings to your target.

## Contributing

The kernel source code uses the akos_ prefix for all public APIs. New code should follow the existing naming conventions and keep board-specific logic isolated within example applications or platform-specific implementations.

## License

Apache License 2.0. See [`LICENSE`](LICENSE).
