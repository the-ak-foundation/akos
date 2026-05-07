# How To Use AKOS

This folder is the quickest way to learn how to bring AKOS up on a board.
Start with [`00-blink`](./00-blink), which is a minimal bare-metal sample for
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
- a sample `Makefile` that includes the AKOS root `Makefile`
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

For a sample to work correctly, the board linker script should include the
kernel fragment between the flash image start and end markers.

In `sample/00-blink/stm32l151xx.ld`, the intended order is:

1. flash start
2. `.text`, `.rodata`, init arrays
3. `INCLUDE "../../kernel/kernel.ld"`
4. `.flash_end`
5. `.data` and `.bss` in RAM

That placement keeps `.task_desc` inside flash, but still before the final flash
boundary marker.

## Build Configuration

The sample `Makefile` should include the repo-root AKOS `Makefile` so it can
reuse the shared kernel and port build settings.

For `sample/00-blink`, the important sample-specific pieces are:

- `include $(abspath ../..)/Makefile`
- `-include $(SAMPLE_DIR)/stm32l1xx.h`
- `$(SAMPLE_DIR)/startup_stm32l151xb.s`
- `$(SAMPLE_DIR)/stm32l151xx.ld`

## `config.h` Setup

The kernel configuration lives in [`kernel/config.h`](../kernel/config.h).
For a sample build, the main things to review are:

- `OS_CFG_SYSTICK_CLOCK_HZ` matches your clock tree
- `OS_CFG_HEAP_SIZE` fits your RAM budget
- `OS_CFG_PRIO_MAX` matches your scheduler design
- `OS_CFG_USE_LOG` and `OS_CFG_USE_CLI` can be overridden from the build
- `OS_CFG_USER_PRINT(...)` points to your print backend

For the blink sample, the Makefile disables logging and CLI by default:

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

## Building A Sample

The `00-blink` sample is self-contained:

- `main.c` shows the AKOS thread registration pattern
- `board.h` shows direct register-level GPIO access
- `stm32l151xx.ld` links the image
- `startup_stm32l151xb.s` provides reset and vector setup
- `Makefile` includes the AKOS root build rules

Build it from the sample directory:

```bash
cd sample/00-blink
make
```

Flash it with:

```bash
make flash
```

## Custom Board Checklist

If you want to move AKOS to a different board, update these pieces:

- `board.h` for pin and clock definitions
- the startup file for the device family
- the linker script for flash and RAM sizes
- `system_*.c` for clock setup
- `main.c` for your thread list and task logic

## Current Sample Pattern

The current `00-blink` sample demonstrates:

- three static AKOS threads
- one thread per LED
- direct GPIO register access
- `akos_thread_delay()` for timing
- root AKOS `Makefile` reuse from the sample `Makefile`
- `kernel/kernel.ld` included between flash start and flash end in the sample linker script
- `kernel/config.h` used for compile-time configuration

That is the recommended pattern for adding new application tasks: make the
task logic small, register it statically, and let AKOS schedule it.
