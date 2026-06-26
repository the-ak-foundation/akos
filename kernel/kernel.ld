/**
  ******************************************************************************
  * @file    kernel.ld
  * @brief   Kernel linker fragment for static thread descriptors.
  *
  * @author  HUYNNG
  * @date    Created: 2026-06-11
  * @date    Updated: 2026-06-26
  *
  * @module  AKOS
  ******************************************************************************
  */

.task_desc : ALIGN(4)
{
    __start_task_desc = .;
    KEEP(*(task_desc))
    __stop_task_desc = .;
} > FLASH
