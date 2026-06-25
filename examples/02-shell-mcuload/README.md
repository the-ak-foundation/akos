# AKOS 02-Shell-MCULoad

This example brings up the built-in AKOS shell on STM32L1 and adds a small
MCU-load demo workload.

What it shows:
- UART-backed AKOS shell on `USART1` at `115200 8N1`
- shell commands layered on top of the built-in `help` and `top`
- one LED thread driven by an AKOS software timer
- three worker threads that create visible CPU load

Useful commands after boot:
- `help`
- `version`
- `mcuload 500`
- `counters`
- `top`
- `led on`
- `led off`
- `led toggle`

Board assumptions:
- STM32L1 image linked at `0x08003000`
- LED on `PB8`
- shell UART on `PA9` / `PA10` using `USART1`

Build and flash:
- `cd examples/02-shell-mcuload`
- `make`
- `make flash`

Notes:
- `mcuload <ticks>` samples busy vs idle time over the given tick window.
- `top` is the live shell view from AKOS itself; press `q` to leave it.
