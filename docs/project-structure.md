# Project structure

```text
.
├── .clang-format
├── .gitignore
├── LICENSE
├── Makefile
├── README.md
├── boards/
│   ├── STM32F030F4P6/
│   └── STM32L151CBT6/
├── docs/
│   └── project-structure.md
├── examples/
│   ├── README.md
│   ├── build.mk
│   ├── message/
│   ├── priority/
│   ├── thread/
│   └── timer/
├── kernel/
│   ├── config.mk
│   ├── inc/
│   ├── kernel.ld
│   ├── kernel.mk
│   └── src/
├── port/
│   ├── ARM/
│   ├── RISCV/
│   └── port.mk
└── third_party/
    ├── README.md
    ├── cmsis/
    ├── third_party.mk
    └── vendor/
```

The root `Makefile` forwards `BOARD` and `EXAMPLE` to the selected example.
Generated files are written to `build/<BOARD>/<EXAMPLE>/` and are not source
files.

## Kernel

```text
kernel/
├── config.mk
├── inc/
│   ├── akos.h
│   ├── config.h
│   ├── core.h
│   ├── list.h
│   ├── log.h
│   ├── memory.h
│   ├── message.h
│   ├── priority.h
│   ├── thread.h
│   └── timer.h
├── kernel.ld
├── kernel.mk
└── src/
    ├── core.c
    ├── list.c
    ├── memory.c
    ├── message.c
    ├── priority.c
    ├── thread.c
    └── timer.c
```

`kernel.mk` loads the kernel configuration, third-party device support, and the
selected CPU port.

## Ports

```text
port/
├── ARM/
│   ├── cortex-m0/
│   │   ├── core_cm0.h
│   │   ├── core_cmFunc.h
│   │   ├── core_cmInstr.h
│   │   ├── port.c
│   │   └── port.h
│   └── cortex-m3/
│       ├── core_cm3.h
│       ├── core_cmFunc.h
│       ├── core_cmInstr.h
│       ├── port.c
│       └── port.h
├── RISCV/
│   └── .gitkeep
└── port.mk
```

`port.mk` selects the implementation from the `ARCH` value supplied by the
board and third-party configuration.

## Boards

```text
boards/
├── STM32F030F4P6/
│   ├── board.c
│   ├── board.h
│   ├── board.mk
│   └── linker.ld
└── STM32L151CBT6/
    ├── board.c
    ├── board.h
    ├── board.mk
    └── linker.ld
```

Each `board.mk` selects `MCU`, applies board configuration, and includes
`kernel/kernel.mk`. Device startup and CMSIS files come from `third_party`.

## Examples

```text
examples/
├── README.md
├── build.mk
├── message/
│   ├── Makefile
│   ├── README.md
│   └── main.c
├── priority/
│   ├── Makefile
│   ├── README.md
│   └── main.c
├── thread/
│   ├── Makefile
│   ├── README.md
│   └── main.c
└── timer/
    ├── Makefile
    ├── README.md
    └── main.c
```

Every example builds independently. Its `Makefile` selects a board and then
includes the shared compiler rules from `examples/build.mk`.

## Third-party software

```text
third_party/
├── README.md
├── third_party.mk
├── cmsis/
│   ├── LICENSE.txt
│   └── Core/
│       └── inc/
│           ├── cmsis_compiler.h
│           ├── cmsis_gcc.h
│           ├── cmsis_version.h
│           ├── core_cm0.h
│           ├── core_cm0plus.h
│           ├── core_cm3.h
│           └── mpu_armv7.h
└── vendor/
    ├── mspm0/
    │   ├── LICENSE.txt
    │   ├── MANIFEST.html
    │   ├── inc/
    │   │   ├── mspm0c110x.h
    │   │   └── ti/devices/
    │   │       ├── DeviceFamily.h
    │   │       └── msp/peripherals/
    │   │           ├── hw_adc12.h
    │   │           ├── hw_crc.h
    │   │           ├── hw_dma.h
    │   │           ├── hw_flashctl.h
    │   │           ├── hw_gpio.h
    │   │           ├── hw_gptimer.h
    │   │           ├── hw_i2c.h
    │   │           ├── hw_iomux.h
    │   │           ├── hw_spi.h
    │   │           ├── hw_uart.h
    │   │           ├── hw_vref.h
    │   │           ├── hw_wuc.h
    │   │           ├── hw_wwdt.h
    │   │           └── m0p/
    │   │               ├── hw_cpuss.h
    │   │               ├── hw_debugss.h
    │   │               ├── hw_factoryregion.h
    │   │               ├── hw_sysctl.h
    │   │               └── sysctl/
    │   │                   └── hw_sysctl_mspm0c110x.h
    │   └── src/
    │       └── startup_mspm0c110x_gcc.c
    ├── stm32f0/
    │   ├── LICENSE.txt
    │   ├── inc/
    │   │   ├── stm32f030x6.h
    │   │   ├── stm32f0xx.h
    │   │   └── system_stm32f0xx.h
    │   └── src/
    │       ├── startup_stm32f030x6.s
    │       └── system_stm32f0xx.c
    └── stm32l1/
        ├── LICENSE.txt
        ├── inc/
        │   ├── stm32l151xb.h
        │   ├── stm32l1xx.h
        │   └── system_stm32l1xx.h
        └── src/
            ├── startup_stm32l151xb.s
            └── system_stm32l1xx.c
```

`third_party.mk` selects CMSIS headers, device definitions, startup code, and
system sources from `MCU`. Upstream versions and licenses are documented in
`third_party/README.md`.
