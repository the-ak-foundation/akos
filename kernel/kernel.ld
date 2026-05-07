/* Kernel linker fragment. */

.task_desc : ALIGN(4)
{
    __start_task_desc = .;
    KEEP(*(task_desc))
    __stop_task_desc = .;
} > FLASH
