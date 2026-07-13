# Message example

The higher-priority slave waits for a signal. The master posts one signal every
500 ticks, waking the slave to toggle the LED and release the message.
The activation bar shows which task is currently running.

```mermaid
sequenceDiagram
    participant Main
    participant Kernel as AKOS Kernel
    participant Slave as Slave (priority 1)
    participant Master as Master (priority 2)
    participant Queue as Slave Queue
    participant Board

    Main->>Kernel: akos_core_init()
    Main->>Kernel: akos_core_run()

    Kernel->>Slave: Start highest-priority task
    activate Slave
    Slave->>Queue: akos_thread_wait_for_msg(FOREVER)
    Queue-->>Slave: Queue empty
    Slave->>Kernel: Block on message
    deactivate Slave

    Kernel->>Master: Schedule master
    activate Master
    Master->>Queue: Post signal 1
    Queue->>Kernel: Slave becomes ready
    deactivate Master
    Kernel->>Slave: Schedule slave
    activate Slave
    Queue-->>Slave: Return message
    Slave->>Board: Toggle LED
    Slave->>Queue: Free message
    Slave->>Queue: Wait for next message
    Slave->>Kernel: Block on message
    deactivate Slave

    Kernel->>Master: Resume master
    activate Master
    Master->>Kernel: akos_thread_delay(500)
    deactivate Master

    Note over Kernel,Master: After 500 ticks the master runs again
```
