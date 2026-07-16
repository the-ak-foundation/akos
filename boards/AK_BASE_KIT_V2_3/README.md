# AK Base Kit V2&3 application board

This target builds AKOS as an application image for the AK Base Kit V2&3 using
the STM32L151CBT6. It has a board-specific BSP and its own CMSIS/device startup
package while reserving the bootloader and BSF regions.

## Flash layout

| Address | Region | Description |
|---|---|---|
| `0x08000000` | Boot | [AK Base Kit V2&3 boot image](https://github.com/ak-embedded-software/ak-base-kit-stm32l151/blob/main/hardware/bin/ak-base-kit-stm32l151-boot.bin) |
| `0x08002000` | BSF | Data shared between Boot and Application |
| `0x08003000` | Application | [AK Base Kit V2&3 application image](https://github.com/ak-embedded-software/ak-base-kit-stm32l151/blob/main/hardware/bin/ak-base-kit-stm32l151-application.bin) |

The linker places `.isr_vector` at `0x08003000` and reserves 116 KiB for the
application. `SystemInit()` relocates `SCB->VTOR` to the same address.

## Board I/O

- Life LED: `PB8`, active-high.
- Console: `USART1` at 115200 baud, TX on `PA9`, RX on `PA10`.

These assignments match the upstream AK Base Kit V2&3
[`io_cfg.h`](https://github.com/the-ak-foundation/ak-base-kit-stm32l151/blob/main/application/sources/platform/stm32l/io_cfg.h)
and [`sys_cfg.h`](https://github.com/the-ak-foundation/ak-base-kit-stm32l151/blob/main/application/sources/platform/stm32l/sys_cfg.h)
platform configuration.

## Build and flash

```bash
make BOARD=AK_BASE_KIT_V2_3 EXAMPLE=thread clean all
```

Run the flash command from the example's build output directory:

```bash
ak-flash /dev/ttyUSB0 akos_ak_base_kit_v2_3_thread.bin 0x08003000
```

Do not program the application image at `0x08000000`, because that would
overwrite the Boot region.
