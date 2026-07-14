# Timer example

This example creates a periodic AKOS software timer with a 500-tick period.
Every expiration posts `MSG_SIGNAL_TIMER` to the Timer Demo task. The task
blocks on its message queue, selects `MSG_SIGNAL_TIMER`, toggles the BSP LED,
prints a timestamped log through `PRINT_DBG()`, frees the message, and waits
again.

```mermaid
sequenceDiagram
    participant Timer as AKOS Timer Task
    participant Kernel as AKOS Kernel
    participant Queue as Timer Demo Queue
    participant App as Timer Demo Task
    participant LED as BSP LED
    participant UART as BSP UART

    loop Every 500 ticks
        Timer->>Queue: Post MSG_SIGNAL_TIMER
        Queue->>Kernel: Timer Demo becomes ready
        Kernel->>App: Schedule Timer Demo
        Queue-->>App: Receive MSG_SIGNAL_TIMER
        App->>LED: Toggle
        App->>UART: PRINT_DBG(timestamped log)
        App->>Kernel: Free message and wait
    end
```

Build from the repository root:

```bash
make BOARD=STM32F103C8T6 EXAMPLE=timer
```
