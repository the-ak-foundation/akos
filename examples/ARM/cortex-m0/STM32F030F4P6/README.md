# STM32F030F4P6 Examples

This folder contains AKOS examples for STM32F030F4P6 Cortex-M0 boards.

## Layout

```text
STM32F030F4P6/
├── Makefile
├── linker/
├── platform/
├── 01-blink_led/
└── 02-button/
```

- `Makefile`: build entry point, compiler flags, flash tool, and link specs.
- `platform/`: startup, system clock helpers, and STM32F0 device headers.
- `linker/`: STM32F030F4P6 linker script.
- `01-blink_led/`: blink PA4 with an AKOS thread.
- `02-button/`: poll PA0 button and toggle PA4 on each stable press.

## Requirements

Install an ARM embedded toolchain and make sure it is in `PATH`:

```sh
arm-none-eabi-gcc --version
arm-none-eabi-objcopy --version
arm-none-eabi-size --version
```

The default flash target uses `st-flash`:

```sh
st-flash --version
```

## Build

Run build commands from the repository root.

Show available apps and targets:

```sh
make -C examples/ARM/cortex-m0/STM32F030F4P6 help
```

Build the default example, `01-blink_led`:

```sh
make -C examples/ARM/cortex-m0/STM32F030F4P6 clean all
```

Build a specific example with `APP=<folder>`:

```sh
make -C examples/ARM/cortex-m0/STM32F030F4P6 APP=01-blink_led clean all
make -C examples/ARM/cortex-m0/STM32F030F4P6 APP=02-button clean all
```

Build only the binary:

```sh
make -C examples/ARM/cortex-m0/STM32F030F4P6 APP=02-button bin
```

Show image size:

```sh
make -C examples/ARM/cortex-m0/STM32F030F4P6 APP=02-button size
```

## Output

Build products are written under `_build/<APP>/`:

```text
_build/01-blink_led/blink_led.elf
_build/01-blink_led/images/blink_led.bin
_build/01-blink_led/images/blink_led.hex
_build/01-blink_led/objects/blink_led.map

_build/02-button/button.elf
_build/02-button/images/button.bin
_build/02-button/images/button.hex
_build/02-button/objects/button.map
```

## Flash

Flash the selected app with:

```sh
make -C examples/ARM/cortex-m0/STM32F030F4P6 APP=01-blink_led flash
make -C examples/ARM/cortex-m0/STM32F030F4P6 APP=02-button flash
```

The default flash address is `0x08000000`. Override it if needed:

```sh
make -C examples/ARM/cortex-m0/STM32F030F4P6 APP=02-button FLASH_ADDR=0x08000000 flash
```

## Add A New Example

Create a new folder and a matching `.mk` file:

```text
03-my_app/
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
make -C examples/ARM/cortex-m0/STM32F030F4P6 APP=03-my_app clean all
```
