# Examples

Build from the repository root:

```sh
make BOARD=STM32F030F4P6 EXAMPLE=thread
make BOARD=STM32L151CBT6 EXAMPLE=timer
```

Or build directly inside an example:

```sh
make -C examples/thread BOARD=STM32F030F4P6
make -C examples/timer BOARD=STM32L151CBT6
```

Available examples: `thread`, `message`, `priority`, and `timer`.
