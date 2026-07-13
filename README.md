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
make help
make BOARD=STM32L151CBT6 EXAMPLE=thread clean all
```

## Examples

The best starting point is [`examples/README.md`](examples/README.md).

Each application in [`examples`](examples) owns its `Makefile`. The root
`Makefile` is only a shortcut that forwards the selected board and example to
that application. Board make fragments select the MCU and include
`kernel/kernel.mk`.

Run `make help` to list the available board and example values.

## Contributing

The kernel source code uses the akos_ prefix for all public APIs. New code should follow the existing naming conventions and keep board-specific logic isolated within example applications or platform-specific implementations.

## License

Apache License 2.0. See [`LICENSE`](LICENSE).
