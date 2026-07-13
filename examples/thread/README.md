# Thread example

This example runs two periodic tasks. Task A has higher priority than Task B;
in AKOS, a smaller priority value means a higher scheduling priority.
The activation bar shows which task is currently running.

```mermaid
sequenceDiagram
    participant Main
    participant Kernel as AKOS Kernel
    participant A as Task A (priority 1)
    participant B as Task B (priority 2)
    participant Idle as Idle Task

    Main->>Kernel: akos_core_init()
    Main->>Kernel: akos_core_run()

    Kernel->>A: Start highest-priority ready task
    activate A
    A->>A: LED ON
    A->>Kernel: akos_thread_delay(500)
    deactivate A

    Kernel->>B: Schedule next ready task
    activate B
    B->>B: LED OFF
    B->>Kernel: akos_thread_delay(1000)
    deactivate B

    Kernel->>Idle: No application task is ready
    activate Idle

    Note over Kernel,A: Tick 500: Task A becomes ready
    deactivate Idle
    Kernel->>A: Preempt Idle
    activate A
    A->>A: LED ON
    A->>Kernel: akos_thread_delay(500)
    deactivate A
    Kernel->>Idle: Resume Idle
    activate Idle

    Note over Kernel,A: Tick 1000: Task A and Task B become ready
    deactivate Idle
    Kernel->>A: Run Task A first
    activate A
    A->>A: LED ON
    A->>Kernel: akos_thread_delay(500)
    deactivate A
    Kernel->>B: Run Task B
    activate B
    B->>B: LED OFF
    B->>Kernel: akos_thread_delay(1000)
    deactivate B
```
