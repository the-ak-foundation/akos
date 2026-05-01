# AKOS — AK Real-Time Operating System

[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)
[![Docs](https://img.shields.io/badge/docs-the--ak--foundation.github.io-green)](https://the-ak-foundation.github.io)

AKOS is an open-source **preemptive Real-Time Operating System** for embedded systems, combining a priority-based preemptive scheduler with an **Event-Driven programming model** (Task / Signal / Timer / State-machine).

Designed for ARM Cortex-M microcontrollers, portable to Linux and custom chipsets.

---

## Architecture

```
┌─────────────────────────────────────────┐
│            Application Layer            │
│     (Tasks, State-machines, Signals)    │
├─────────────────────────────────────────┤
│         Event-Driven Framework          │
│    task_post() · timer_set() · msg_q    │
├──────────────┬──────────────────────────┤
│  Preemptive  │   Cooperative fallback   │
│  Scheduler   │   (config option)        │
├──────────────┴──────────────────────────┤
│          Kernel Core (C/C++)            │
│  Context switch · IRQ · Memory · Tick   │
├─────────────────────────────────────────┤
│        Architecture Port Layer          │
│    ARM Cortex-M3 · Linux/POSIX (WIP)    │
├─────────────────────────────────────────┤
│    HAL / BSP (Hardware Abstraction)     │
│  STM32L151 · AK Embedded Base Kit v3    │
└─────────────────────────────────────────┘
```

## Supported hardware

| Board | MCU | Status |
|-------|-----|--------|
| AK Embedded Base Kit v3 | STM32L151C8T6 (ARM Cortex-M3) | ✅ Primary target |
| Linux / POSIX | Any | 🚧 In progress |

## Quick start

**Prerequisites:**
```bash
sudo apt-get install gcc-arm-none-eabi cmake ninja-build
```

**Build the blink sample:**
```bash
git clone https://github.com/the-ak-foundation/akos
cd akos
cmake -B build -DAKOS_BOARD=ak-base-kit-v3 -DAKOS_SAMPLE=00-blink
cmake --build build
```

**Flash to hardware:**
```bash
# Using ak-flash
ak_flash /dev/ttyUSB0 build/akos-00-blink.bin 0x08003000
```

## Repository structure

```
akos/
├── kernel/
│   ├── core/          # Scheduler, task, signal, timer, message queue
│   └── port/
│       └── arm-cortex-m3/   # PendSV context switch, SysTick
├── arch/
│   └── arm-cortex-m3/       # Startup, vector table, linker scripts
├── hal/
│   └── stm32l151/           # GPIO, UART, SPI, I2C, TIM, Flash drivers
├── bsp/
│   └── ak-base-kit-v3/      # Board init, pin mapping, clock config
├── include/
│   └── akos/                # Public API headers
├── samples/
│   ├── 00-blink/            # Hello world — LED blink
│   ├── 01-task-signal/      # Task + Signal basics
│   └── 02-timer-periodic/   # Periodic timer
├── tests/                   # Unit tests (host-native)
└── cmake/                   # Toolchain files, helper functions
```

## Key concepts

AKOS uses an **event-driven model** on top of a preemptive kernel:

```c
// Define a task
void app_task_handler(ak_msg_t *msg) {
    switch (msg->sig) {
    case SIG_INIT:
        timer_set(APP_TASK_ID, SIG_BLINK, 500, TIMER_PERIODIC);
        break;
    case SIG_BLINK:
        hal_gpio_toggle(LED_PIN);
        break;
    }
}
```

No `while(1)`, no blocking delays. Tasks communicate via signals — the scheduler handles preemption transparently.

## Contributing

See [CONTRIBUTING.md](https://github.com/the-ak-foundation/.github/blob/main/CONTRIBUTING.md) and the [documentation](https://the-ak-foundation.github.io/docs/porting).

## License

Apache License 2.0 — see [LICENSE](LICENSE).
