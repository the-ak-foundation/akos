# STM32L151CBT6 board

Board support for the STM32L151CBT6 (Arm Cortex-M3), configured to run at
32 MHz.

## Contents

- `bsp.c`, `bsp.h`: clock and board I/O initialization.
- `board.mk`: compiler definitions, sources, and include paths.
- `linker.ld`: memory layout for the STM32L151CBT6.
- `third_party/cmsis/`: minimal Arm CMSIS-Core 5.9.0 files required by Cortex-M3.
- `third_party/device/`: STM32L1 CMSIS Device v2.3.4 headers, system source, and startup.

The files under `third_party/` retain their upstream licenses. See
`third_party/cmsis/LICENSE.txt` and `third_party/device/LICENSE.txt`.

| Component | Upstream release | Source | Local license |
|---|---|---|---|
| Arm CMSIS-Core | 5.9.0 (`2b7495b8535bdcb306dac29b9ded4cfb679d7e5c`) | <https://github.com/ARM-software/CMSIS_5> | [`third_party/cmsis/LICENSE.txt`](third_party/cmsis/LICENSE.txt) |
| STM32L1 CMSIS Device | v2.3.4 (`69ed99e32827a72ea9594c081fb3efc7d47ef723`) | <https://github.com/STMicroelectronics/cmsis_device_l1> | [`third_party/device/LICENSE.txt`](third_party/device/LICENSE.txt) |
