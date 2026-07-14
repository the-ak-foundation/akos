# Thread example

This example follows the AKOS `00-blink` behavior: three independent thread
descriptors run one shared `blink_task()` with different context objects. The
contexts select periods of 100, 200, and 400 scheduler ticks.

The current portable BSP exposes one board LED, so all three task instances
toggle that LED. Each action is logged through the common `PRINT_DBG()` helper
and BSP UART at 115200 baud.

```mermaid
sequenceDiagram
    participant Kernel as AKOS Kernel
    participant T100 as Blink 100 ms
    participant T200 as Blink 200 ms
    participant T400 as Blink 400 ms
    participant LED as BSP LED
    participant UART as BSP UART

    par Independent periodic threads
        loop Every 100 ticks
            Kernel->>T100: Schedule
            T100->>LED: Toggle
            T100->>UART: PRINT_DBG(toggle log)
            T100->>Kernel: Delay 100 ticks
        end
    and
        loop Every 200 ticks
            Kernel->>T200: Schedule
            T200->>LED: Toggle
            T200->>UART: PRINT_DBG(toggle log)
            T200->>Kernel: Delay 200 ticks
        end
    and
        loop Every 400 ticks
            Kernel->>T400: Schedule
            T400->>LED: Toggle
            T400->>UART: PRINT_DBG(toggle log)
            T400->>Kernel: Delay 400 ticks
        end
    end
```

Build from the repository root:

```bash
make BOARD=STM32F103C8T6 EXAMPLE=thread
```
