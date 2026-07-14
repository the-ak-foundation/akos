# STM32F030F4P6 board

Board support for the STM32F030F4P6 (Arm Cortex-M0), configured to run at
48 MHz. The life LED is connected to PA4.

## Contents

- `bsp.c`, `bsp.h`: clock and board I/O initialization.
- `board.mk`: compiler definitions, sources, and include paths.
- `linker.ld`: memory layout for the STM32F030F4P6.
- `third_party/cmsis/`: minimal Arm CMSIS-Core 5.9.0 files required by Cortex-M0.
- `third_party/device/`: STM32F0 CMSIS Device v2.3.7 headers, system source, and startup.

The files under `third_party/` retain their upstream licenses. See
`third_party/cmsis/LICENSE.txt` and `third_party/device/LICENSE.txt`.

| Component | Upstream release | Source | Local license |
|---|---|---|---|
| Arm CMSIS-Core | 5.9.0 (`2b7495b8535bdcb306dac29b9ded4cfb679d7e5c`) | <https://github.com/ARM-software/CMSIS_5> | [`third_party/cmsis/LICENSE.txt`](third_party/cmsis/LICENSE.txt) |
| STM32F0 CMSIS Device | v2.3.7 (`3973d99a89ac5fdf1f2593abc29a73d2981ce94a`) | <https://github.com/STMicroelectronics/cmsis_device_f0> | [`third_party/device/LICENSE.txt`](third_party/device/LICENSE.txt) |
