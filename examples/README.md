# How To Use AKOS Examples

This folder is the quickest way to learn how to bring AKOS up on a board. The
examples are organized by CPU family and board:

```text
examples/
└── ARM/
    ├── cortex-m0/
    │   ├── LP-MSPM0C1104/
    │   └── STM32F030F4P6/
    └── cortex-m3/
        └── STM32L151CBT6/
```

## The AKOS Flow

AKOS is built around a small runtime sequence:

1. bring up the board
2. register static threads
3. initialize the kernel
4. start the scheduler
5. let threads run and block with `akos_thread_delay()`

The public kernel entry points live in [`kernel/inc/akos.h`](../kernel/inc/akos.h).

## Build Pattern

Each board folder owns:

- a board `Makefile`
- a `platform/` folder for startup, clock, vector, and device support
- a `linker/` folder for the board memory map
- one or more app folders, each with a small `.mk` file and `main.c`

The board `Makefile` includes [`kernel/kernel.mk`](../kernel/kernel.mk), which
adds kernel sources, kernel include paths, common config defines, and the
selected CPU port through [`port/port.mk`](../port/port.mk).

Run `help` in any board folder to see supported apps and targets:

```sh
make -C examples/ARM/cortex-m3/STM32L151CBT6 help
make -C examples/ARM/cortex-m0/STM32F030F4P6 help
make -C examples/ARM/cortex-m0/LP-MSPM0C1104 help
```

## Linker Layout

For an example to work correctly, the board linker script should include the
kernel linker fragment between the flash image start and end markers.

The intended order is:

1. flash start
2. `.text`, `.rodata`, init arrays
3. `INCLUDE "kernel.ld"`
4. `.flash_end`
5. `.data` and `.bss` in RAM

The board Makefile gets the fragment search path from `KERNEL_LDFLAGS`, exported
by `kernel/kernel.mk`:

```make
KERNEL_LDFLAGS := \
	-Wl,-L$(KERNEL_DIR)
```

That keeps board linker scripts independent from the repository-relative
location of the kernel folder.

## Build Configuration

The default kernel configuration lives in
[`kernel/inc/config.h`](../kernel/inc/config.h). Build-time overrides live in
[`kernel/config.mk`](../kernel/config.mk), and board Makefiles can override
values before including `kernel/kernel.mk`.

Review these values when adding or moving a board:

- `OS_CFG_SYSTICK_CLOCK_HZ` matches your clock tree
- `OS_CFG_HEAP_SIZE` fits your RAM budget
- `OS_CFG_PRIO_MAX` matches your scheduler design
- `OS_CFG_USE_LOG` and `OS_CFG_USE_CLI` match the example needs
- `OS_CFG_USER_PRINT(...)` points to your print backend when logging is enabled

## Minimal Thread Template

```c
#include "akos.h"

static void worker(void *p_arg)
{
    (void)p_arg;

    for (;;)
    {
        akos_thread_delay(100u);
    }
}

AKOS_THREAD_DEFINE(worker_desc, 0u, worker, NULL, 4u, 0u, 128u);

int main(void)
{
    akos_core_init();
    akos_core_run();

    for (;;)
    {
    }
}
```

## Current Boards

```text
examples/
└── ARM/
    ├── cortex-m0/
    │   ├── LP-MSPM0C1104/
    │   └── STM32F030F4P6/
    └── cortex-m3/
        └── STM32L151CBT6/
```

Build from the repository root:

```sh
make -C examples/ARM/cortex-m3/STM32L151CBT6 clean all
make -C examples/ARM/cortex-m0/STM32F030F4P6 APP=<app> clean all
make -C examples/ARM/cortex-m0/LP-MSPM0C1104 clean all
```

## Custom Board Checklist

If you want to move AKOS to a different board, update these pieces:

- the board Makefile and `ARCH` setting
- the platform startup file and vector table
- the linker script for flash and RAM sizes
- clock setup in the platform layer
- app `main.c` files for your thread list and task logic
