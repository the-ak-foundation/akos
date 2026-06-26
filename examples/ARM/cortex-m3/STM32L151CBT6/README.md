# STM32L151CBT6 Examples

This folder contains AKOS examples for STM32L151CBT6 Cortex-M3 boards.

## Layout

```text
STM32L151CBT6/
├── Makefile
├── linker/
├── platform/
└── 01-blink/
```

- `Makefile`: common build entry point for this board.
- `platform/`: startup, system clock setup, and STM32L1 device header.
- `linker/`: STM32L151CBT6 linker script.
- `01-blink/`: three LED blink threads.

## Build

Run build commands from the repository root.

Show available apps and targets:

```sh
make -C examples/ARM/cortex-m3/STM32L151CBT6 help
```

Build the default app:

```sh
make -C examples/ARM/cortex-m3/STM32L151CBT6 clean all
```

Build a specific app:

```sh
make -C examples/ARM/cortex-m3/STM32L151CBT6 APP=01-blink clean all
```

## Output

Build products are written under `_build/<APP>/`:

```text
_build/01-blink/blink.elf
_build/01-blink/images/blink.bin
_build/01-blink/images/blink.hex
_build/01-blink/objects/blink.map
```

## Flash

This example is linked at `0x08003000`, matching the original bootloader-offset
layout.

```sh
make -C examples/ARM/cortex-m3/STM32L151CBT6 APP=01-blink flash
```

Override the flash address when needed:

```sh
make -C examples/ARM/cortex-m3/STM32L151CBT6 APP=01-blink FLASH_ADDR=0x08003000 flash
```
