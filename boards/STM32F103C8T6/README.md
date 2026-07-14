# STM32F103C8T6 board

Board support package for the STM32F103C8T6 Blue Pill (Arm Cortex-M3).
It uses an 8 MHz HSE to run at 72 MHz, the active-low LED on PC13, and USART1
on PA9/PA10 at 115200 baud.

Board initialization and peripheral APIs are exposed through `bsp.h` and
`bsp.c`.

| Component | Upstream release | Source | Local license |
|---|---|---|---|
| Arm CMSIS-Core | 5.9.0 (`2b7495b8535bdcb306dac29b9ded4cfb679d7e5c`) | <https://github.com/ARM-software/CMSIS_5> | [`third_party/cmsis/LICENSE.txt`](third_party/cmsis/LICENSE.txt) |
| STM32F1 CMSIS Device | v4.3.5 (`8a76309ed1250d817e9c888c4417171d2ba3ba63`) | <https://github.com/STMicroelectronics/cmsis-device-f1> | [`third_party/device/LICENSE.md`](third_party/device/LICENSE.md) |
