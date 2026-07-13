# Priority example

Task High has priority 1 and Task Low has priority 2. AKOS always schedules the
ready task with the smaller priority value first.
The activation bar shows which task is currently running.

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
    activate High
    High->>High: Toggle LED
    High->>Kernel: akos_thread_delay(100)
    deactivate High

    Kernel->>Low: Schedule next ready task
    activate Low
    Low->>Kernel: akos_thread_delay(1000)
    deactivate Low
    Kernel->>Idle: No application task is ready
    activate Idle

    Note over Kernel,High: Tick 100: Task High becomes ready
    deactivate Idle
    Kernel->>High: Preempt Idle
    activate High
    High->>High: Toggle LED
    High->>Kernel: akos_thread_delay(100)
    deactivate High
    Kernel->>Idle: Resume Idle
    activate Idle

    Note over Kernel,Low: Tick 1000: Task High and Task Low are ready
    deactivate Idle
    Kernel->>High: Run higher-priority task first
    activate High
    High->>High: Toggle LED
    High->>Kernel: akos_thread_delay(100)
    deactivate High
    Kernel->>Low: Run lower-priority task
    activate Low
    Low->>Kernel: akos_thread_delay(1000)
    deactivate Low
```
