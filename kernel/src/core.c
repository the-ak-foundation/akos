/**
 * @file core.c
 * @brief Kernel initialization, startup and critical-section control.
 */

#include "core.h"

#include "message.h"
#include "timer.h"
#include "priority.h"
#include "thread.h"
#include "log.h"
#include "config.h"
#include "port.h"

static uint16_t critical_nesting_count = (uint16_t)0u;

/**
 * @brief Enter nested critical section.
 */
void akos_core_enter_critical(void)
{
    port_disable_interrupts
    critical_nesting_count++;
}

/**
 * @brief Exit nested critical section.
 */
void akos_core_exit_critical(void)
{
    core_assert(critical_nesting_count, "NESTING CRITICAL UNBALANCED");
    critical_nesting_count--;
    if (critical_nesting_count == 0)
    {
        port_enable_interrupts
    }
}

/**
 * @brief Initialize kernel modules.
 */
void akos_core_init(void)
{
    akos_port_systick_init_freq(OS_CFG_SYSTICK_CLOCK_HZ);
    akos_priority_init();
    akos_message_init();
    akos_timer_init();
    akos_thread_register_static_threads();
}

/**
 * @brief Start scheduler and switch to the first task.
 */
void akos_core_run(void)
{
    akos_thread_start();
    port_setup_PendSV();
    akos_port_start_first_task();
}
