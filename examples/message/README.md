# Message example

The higher-priority slave waits for a signal. The master posts one signal every
500 ticks, waking the slave to toggle the LED and release the message.

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
    Slave->>Queue: akos_thread_wait_for_msg(FOREVER)
    Queue-->>Slave: Queue empty
    Slave->>Kernel: Block on message

    Kernel->>Master: Schedule master
    Master->>Queue: Post signal 1
    Queue->>Kernel: Slave becomes ready
    Kernel->>Slave: Schedule slave
    Queue-->>Slave: Return message
    Slave->>Board: Toggle LED
    Slave->>Queue: Free message
    Slave->>Queue: Wait for next message
    Slave->>Kernel: Block on message

    Kernel->>Master: Resume master
    Master->>Kernel: akos_thread_delay(500)

    Note over Kernel,Master: After 500 ticks the master runs again
```
