# Priority example

This example demonstrates AKOS priority scheduling with two independent tasks:

- Task High has priority 1 and receives a periodic signal every 100 ticks.
- Task Low has priority 2 and receives a periodic signal every 1000 ticks.

A smaller priority number means a higher scheduling priority. When both tasks
receive a timer signal together, AKOS runs Task High first. Both tasks block on
their message queues between signals. All logs use the common `PRINT_DBG()`
helper and BSP UART at 115200 baud.

```mermaid
sequenceDiagram
    participant Timer as AKOS Timer Task
    participant Kernel as AKOS Kernel
    participant HighQueue as High Queue
    participant High as Task High (priority 1)
    participant LowQueue as Low Queue
    participant Low as Task Low (priority 2)
    participant LED as BSP LED
    participant UART as BSP UART

    Note over Timer,Low: At tick 1000 both periodic timers expire
    Timer->>HighQueue: Post MSG_SIGNAL_PRIO_HIGH
    Timer->>LowQueue: Post MSG_SIGNAL_PRIO_LOW
    HighQueue->>Kernel: Task High becomes ready
    LowQueue->>Kernel: Task Low becomes ready

    Kernel->>High: Schedule priority 1 first
    HighQueue-->>High: Receive high signal
    High->>LED: Toggle
    High->>UART: Print timestamped high log
    High->>Kernel: Free message and wait

    Kernel->>Low: Schedule priority 2 next
    LowQueue-->>Low: Receive low signal
    Low->>UART: Print timestamped low log
    Low->>Kernel: Free message and wait
```

Build from the repository root:

```bash
make BOARD=STM32F103C8T6 EXAMPLE=priority
```
