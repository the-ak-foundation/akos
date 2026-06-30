<div align="center">
  
![Repo Traffic](https://komarev.com/ghpvc/?username=akos&label=Repo+Traffic&color=blue&style=flat-square)

</div>

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
| [AK Embedded Base Kit](https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu) | STM32L151C8T6 (ARM Cortex-M3) | Primary target |

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
cd examples/00-blink
make
make flash
```

## Examples

The best starting point is [`examples/README.md`](examples/README.md).

The current example layout includes:

- [`examples/00-blink`](examples/00-blink) for a minimal bare-metal STM32L1 bring-up
- Static thread registration with `AKOS_THREAD_DEFINE(...)`
- An example `Makefile` that includes the repo-root `Makefile`
- An example linker script that includes `kernel/kernel.ld` between flash start and flash end
- Example configuration guidance in `kernel/config.h`

If you are bringing up a new board, start with `examples/00-blink` and adapt the board file, linker script, startup file, and `config.h` settings to your target.

## Contributing

The kernel source code uses the akos_ prefix for all public APIs. New code should follow the existing naming conventions and keep board-specific logic isolated within example applications or platform-specific implementations.

## License

Apache License 2.0. See [`LICENSE`](LICENSE).
