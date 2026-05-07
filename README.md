# AKOS — AK Real-Time Operating System

[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)
[![Docs](https://img.shields.io/badge/docs-the--ak--foundation.github.io-green)](https://the-ak-foundation.github.io)

AKOS is an open-source preemptive real-time operating system for embedded
systems. It combines a priority-based scheduler with an event-driven program
model built around tasks, signals, timers, and state machines.

AKOS is designed for ARM Cortex-M microcontrollers and can be adapted to other
targets.

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
## Supported hardware

| Board | MCU | Status |
|-------|-----|--------|
| AK Embedded Base Kit v3 | STM32L151C8T6 (ARM Cortex-M3) | ✅ Primary target |

## Quick start

**Prerequisites:**
```bash
sudo apt-get install gcc-arm-none-eabi cmake ninja-build
```

**Build the blink sample on AK Embedded Base Kit v3 :**
```bash
git clone https://github.com/the-ak-foundation/akos
cd akos
cd sample/00-blink
make
make flash
```

## Samples

The best starting point is [`sample/README.md`](sample/README.md).

The current sample layout includes:

- [`sample/00-blink`](sample/00-blink) for a minimal bare-metal STM32L1 bring-up
- static thread registration with `AKOS_THREAD_DEFINE(...)`
- a sample `Makefile` that includes the repo-root `Makefile`
- a sample linker script that includes `kernel/kernel.ld` between flash start
  and flash end
- sample configuration guidance in `kernel/config.h`

If you are bringing up a new board, start with `sample/00-blink` and adapt the
board file, linker script, startup file, and `config.h` settings to your target.

## Contributing

The kernel source uses `akos_` prefixes for public APIs. New code should follow
the existing naming style and keep board-specific logic in sample or platform
code.

## License

Apache License 2.0. See [`LICENSE`](LICENSE).
