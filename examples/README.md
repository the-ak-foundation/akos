# Examples

Available examples:

- **01 - Thread:** Create multiple threads that run one shared task with different contexts and periods.
- **02 - Message:** Send and validate dynamic messages between a master thread and a slave thread.
- **03 - Priority:** Observe how AKOS schedules ready threads according to their priorities.
- **04 - Timer:** Use a periodic software timer to deliver signals to an application thread.

Use `BOARD=<board>` to build for another supported board.

| Example | Build from root | Link |
|---|---|---|
| Thread | `make BOARD=AK_BASE_KIT_V2_3 EXAMPLE=thread` | [README](thread/README.md) |
| Message | `make BOARD=AK_BASE_KIT_V2_3 EXAMPLE=message` | [README](message/README.md) |
| Priority | `make BOARD=AK_BASE_KIT_V2_3 EXAMPLE=priority` | [README](priority/README.md) |
| Timer | `make BOARD=AK_BASE_KIT_V2_3 EXAMPLE=timer` | [README](timer/README.md) |
