# How To Use AKOS

This folder is the quickest way to learn how to bring AKOS up on a board.
Start with [`00-blink`](./00-blink), which is a minimal bare-metal example for
STM32L1.

## The AKOS Flow

AKOS is built around a small runtime sequence:

1. bring up the board
2. register static threads
3. initialize the kernel
4. start the scheduler
5. let threads run and block with `akos_thread_delay()`

The public kernel entry points live in [`akos.h`](../akos.h).

## What Your Application Must Provide

Your app typically provides:

- a startup file
- a linker script
- an example `Makefile` that includes the AKOS root `Makefile`
- a `main()` function
- a board helper layer for GPIO, clock, UART, or other peripherals
- one or more static thread descriptors

For static registration, use:

```c
AKOS_THREAD_DEFINE(name, id, entry, arg, prio, queue_size, stack_size);
```

Each definition lands in the linker section named `task_desc`. The kernel
collects those descriptors during `akos_core_init()`.

## Linker Layout

For an example to work correctly, the board linker script should include the
kernel fragment between the flash image start and end markers.

In `examples/00-blink/stm32l151xx.ld`, the intended order is:

1. flash start
2. `.text`, `.rodata`, init arrays
3. `INCLUDE "../../kernel/kernel.ld"`
4. `.flash_end`
5. `.data` and `.bss` in RAM

That placement keeps `.task_desc` inside flash, but still before the final flash
boundary marker.

## Build Configuration

The example `Makefile` should include the repo-root AKOS `Makefile` so it can
reuse the shared kernel and port build settings.

For `examples/00-blink`, the important example-specific pieces are:

- `include $(abspath ../..)/Makefile`
- `-include $(SAMPLE_DIR)/stm32l1xx.h`
- `$(SAMPLE_DIR)/startup_stm32l151xb.s`
- `$(SAMPLE_DIR)/stm32l151xx.ld`

## `config.h` Setup

The kernel configuration lives in [`kernel/config.h`](../kernel/config.h).
For an example build, the main things to review are:

- `OS_CFG_SYSTICK_CLOCK_HZ` matches your clock tree
- `OS_CFG_HEAP_SIZE` fits your RAM budget
- `OS_CFG_PRIO_MAX` matches your scheduler design
- `OS_CFG_USE_LOG` and `OS_CFG_USE_CLI` can be overridden from the build
- `OS_CFG_USER_PRINT(...)` points to your print backend

For the blink example, the Makefile disables logging and CLI by default:

```make
-DOS_CFG_USE_LOG=0 -DOS_CFG_USE_CLI=0
```

If you want logging, set `OS_CFG_USER_PRINT(...)` to your UART/SWO print
function or keep the default `printf(...)` backend for hosted testing.

## Minimal Thread Template

```c
#include "akos.h"

static void worker(void *p_arg)
{
    (void)p_arg;

    for (;;)
    {
        /* do work */
        akos_thread_delay(100u);
    }
}

AKOS_THREAD_DEFINE(worker_desc, 0u, worker, NULL, 4u, 0u, 128u);

int main(void)
{
    /* board_init(); */
    akos_core_init();
    akos_core_run();

    for (;;)
    {
        /* akos_core_run() should not return */
    }
}
```

## Building An Example

The `00-blink` example is self-contained:

- `main.c` shows the AKOS thread registration pattern
- `board.h` shows direct register-level GPIO access
- `stm32l151xx.ld` links the image
- `startup_stm32l151xb.s` provides reset and vector setup
- `Makefile` includes the AKOS root build rules

Build it from the example directory:

```bash
cd examples/00-blink
make
```

This produces `akos-00-blink.bin`. Flash it to the bootloader offset with
[`ak-flash`](https://github.com/the-ak-foundation/ak-flash):

```bash
ak-flash /dev/ttyUSB0 akos-00-blink.bin 0x08003000
```

If your serial device is different, replace `/dev/ttyUSB0` with the correct
port for your board.

## Custom Board Checklist

If you want to move AKOS to a different board, update these pieces:

- `board.h` for pin and clock definitions
- the startup file for the device family
- the linker script for flash and RAM sizes
- `system_*.c` for clock setup
- `main.c` for your thread list and task logic

## Current Sample Pattern

The current `00-blink` example demonstrates:

- three static AKOS threads
- one thread per LED
- direct GPIO register access
- `akos_thread_delay()` for timing
- root AKOS `Makefile` reuse from the example `Makefile`
- `kernel/kernel.ld` included between flash start and flash end in the example linker script
- `kernel/config.h` used for compile-time configuration

That is the recommended pattern for adding new application tasks: make the
task logic small, register it statically, and let AKOS schedule it.
