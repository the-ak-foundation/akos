# LP-MSPM0C1104 Examples

This folder contains AKOS examples for the TI LP-MSPM0C1104 Cortex-M0+ LaunchPad.

## Layout

```text
LP-MSPM0C1104/
├── Makefile
├── linker/
├── platform/
└── 01-blink_led/
```

- `Makefile`: build entry point, compiler flags, and link specs.
- `platform/`: startup, fixed system clock helpers, and TI MSPM0C110x CMSIS headers.
- `linker/`: MSPM0C1104 linker script for 16 KB flash and 1 KB SRAM.
- `01-blink_led/`: blink GPIOA DIO22 with an AKOS thread.

## Requirements

Install an ARM embedded toolchain and make sure it is in `PATH`:

```sh
arm-none-eabi-gcc --version
arm-none-eabi-objcopy --version
arm-none-eabi-size --version
```

The examples are built for a Cortex-M0+ target using the bundled TI MSPM0C110x
CMSIS device headers.

## Build

Run build commands from the repository root.

Show available apps and targets:

```sh
make -C examples/ARM/cortex-m0/LP-MSPM0C1104 help
```

Build the default example, `01-blink_led`:

```sh
make -C examples/ARM/cortex-m0/LP-MSPM0C1104 clean all
```

Build a specific example with `APP=<folder>`:

```sh
make -C examples/ARM/cortex-m0/LP-MSPM0C1104 APP=01-blink_led clean all
```

Build only the binary:

```sh
make -C examples/ARM/cortex-m0/LP-MSPM0C1104 APP=01-blink_led bin
```

Show image size:

```sh
make -C examples/ARM/cortex-m0/LP-MSPM0C1104 APP=01-blink_led size
```

## Clean

Remove intermediate files while keeping `.bin` and `.hex` artifacts:

```sh
make -C examples/ARM/cortex-m0/LP-MSPM0C1104 APP=01-blink_led clean
```

## Output

Build products are written under `_build/<APP>/`:

```text
_build/01-blink_led/blink_led.elf
_build/01-blink_led/images/blink_led.bin
_build/01-blink_led/images/blink_led.hex
_build/01-blink_led/objects/blink_led.map
```

## Flash

This board Makefile currently builds the firmware image only. It does not define
a `flash` target yet.

Use your MSPM0 programming flow, such as TI UniFlash, Code Composer Studio, or a
debug probe workflow, to program the generated image:

```text
examples/ARM/cortex-m0/LP-MSPM0C1104/_build/01-blink_led/blink_led.elf
examples/ARM/cortex-m0/LP-MSPM0C1104/_build/01-blink_led/images/blink_led.bin
examples/ARM/cortex-m0/LP-MSPM0C1104/_build/01-blink_led/images/blink_led.hex
```

The linker script places the image at flash origin `0x00000000`.

## Add A New Example

Create a new folder and a matching `.mk` file:

```text
02-my_app/
├── my_app.mk
├── board.h
└── main.c
```

The `.mk` file should set `APP_TARGET` and `APP_SOURCES`:

```make
APP_TARGET := my_app

APP_SOURCES := \
	$(APP_DIR)/main.c
```

Build it with:

```sh
make -C examples/ARM/cortex-m0/LP-MSPM0C1104 APP=02-my_app clean all
```
