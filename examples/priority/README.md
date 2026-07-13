# Priority example

Task High has priority 1 and Task Low has priority 2. AKOS always schedules the
ready task with the smaller priority value first.

```mermaid
sequenceDiagram
    participant Main
    participant Kernel as AKOS Kernel
    participant High as Task High (priority 1)
    participant Low as Task Low (priority 2)
    participant Idle as Idle Task

    Main->>Kernel: akos_core_init()
    Main->>Kernel: akos_core_run()

    Kernel->>High: Start highest-priority ready task
    High->>High: Toggle LED
    High->>Kernel: akos_thread_delay(100)

    Kernel->>Low: Schedule next ready task
    Low->>Kernel: akos_thread_delay(1000)
    Kernel->>Idle: No application task is ready

    Note over Kernel,High: Tick 100: Task High becomes ready
    Kernel->>High: Preempt Idle
    High->>High: Toggle LED
    High->>Kernel: akos_thread_delay(100)
    Kernel->>Idle: Resume Idle

    Note over Kernel,Low: Tick 1000: Task High and Task Low are ready
    Kernel->>High: Run higher-priority task first
    High->>High: Toggle LED
    High->>Kernel: akos_thread_delay(100)
    Kernel->>Low: Run lower-priority task
    Low->>Kernel: akos_thread_delay(1000)
```
