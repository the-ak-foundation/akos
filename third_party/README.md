# Third-party software

This directory contains unmodified files imported from the official upstream
repositories. AKOS's Apache-2.0 license does not replace the licenses shipped
with these components.

| Component | Upstream release | Source | Local license |
|---|---|---|---|
| Arm CMSIS-Core | 5.9.0 (`2b7495b8535bdcb306dac29b9ded4cfb679d7e5c`) | <https://github.com/ARM-software/CMSIS_5> | [`cmsis/LICENSE.txt`](cmsis/LICENSE.txt) |
| STM32F0 CMSIS Device | v2.3.7 (`3973d99a89ac5fdf1f2593abc29a73d2981ce94a`) | <https://github.com/STMicroelectronics/cmsis_device_f0> | [`vendor/stm32f0/LICENSE.txt`](vendor/stm32f0/LICENSE.txt) |
| STM32L1 CMSIS Device | v2.3.4 (`69ed99e32827a72ea9594c081fb3efc7d47ef723`) | <https://github.com/STMicroelectronics/cmsis_device_l1> | [`vendor/stm32l1/LICENSE.txt`](vendor/stm32l1/LICENSE.txt) |
| TI MSPM0 SDK | 2.10.00.04 (`e249e2bd63bcc912176a30a45a6a5dcea951168b`) | <https://github.com/TexasInstruments/mspm0-sdk> | [`vendor/mspm0/LICENSE.txt`](vendor/mspm0/LICENSE.txt) and [`vendor/mspm0/MANIFEST.html`](vendor/mspm0/MANIFEST.html) |

Only files needed by the targets documented in `docs/project-structure.md` are
vendored. File contents and copyright headers are preserved from upstream.

## Make integration

Include `third_party/third_party.mk` and set `MCU` to one of:

- `STM32F030F4P6`
- `STM32L151CBT6`
- `MSPM0C1104`

The make fragment exports include paths, device defines, source paths, and
`VPATH` entries through the standard `CPPFLAGS`, `SOURCES`, and `VPATH`
variables. If `kernel/kernel.mk` is used, the fragment is included
automatically.
