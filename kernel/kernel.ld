/*!
 * @file	kernel.ld
 * @author	Snoopy3921 - AK Foundation
 * @date	2026/05/08
 * @module	AKOS
 * @brief	Kernel linker fragment.
 */

.task_desc : ALIGN(4)
{
    __start_task_desc = .;
    KEEP(*(task_desc))
    __stop_task_desc = .;
} > FLASH
