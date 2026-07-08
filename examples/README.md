# AKOS Examples

This folder contains small board examples for bringing up AKOS.

## Prerequisites

To build an AKOS application, provide these pieces:

- a board startup file with the vector table and reset handler
- a linker script that includes `kernel/kernel.ld`
- a device or CMSIS header path for the selected MCU
- a board support layer for clock, GPIO, UART, or other peripherals used by the app
- a `Makefile` that includes the AKOS root `Makefile`
- at least one static thread defined with `AKOS_THREAD_DEFINE(...)`
- any required config overrides, such as `AKOS_CFG_SYSTICK_CLOCK_HZ` and pool sizes

| Example | Target | Description |
| --- | --- | --- |
| [`00-blink`](./00-blink/README.md) | Cortex-M3 / STM32L1 | Minimal three-thread LED blink example. |
| [`01-blink-cm0`](./01-blink-cm0/README.md) | Cortex-M0 / STM32F0 | Minimal single-thread LED blink example. |
| [`02-shell-mcuload`](./02-shell-mcuload/README.md) | Cortex-M3 / STM32L1 | UART shell with MCU-load commands and worker threads. |

Each example README includes its own board assumptions, build notes, and usage.

## Build And Flash

Build an example from its own directory:

```bash
cd examples/00-blink
make
```

Flash it with the example `Makefile` target:

```bash
make flash
```

If your board uses the AK bootloader over serial, pass the device path:

```bash
make flash dev=/dev/ttyUSB0
```

Use the same pattern for the other examples:

```bash
cd examples/01-blink-cm0
make
make flash
make flash dev=/dev/ttyUSB0

cd examples/02-shell-mcuload
make
make flash
make flash dev=/dev/ttyUSB0
```
