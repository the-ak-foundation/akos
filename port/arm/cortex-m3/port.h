#ifndef PORT_H
#define PORT_H

/**
 * @file port.h
 * @brief Cortex-M3 port API for AKOS.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure SysTick to generate 1ms OS ticks.
 * @param cpu_freq Core clock frequency in Hz.
 */
void akos_port_systick_init_freq(uint32_t cpu_freq);

/**
 * @brief Start the first thread by restoring the initial exception context.
 */
void akos_port_start_first_task(void);

/**
 * @brief Build the initial Cortex-M thread stack frame.
 * @param p_stack Base stack buffer.
 * @param stack_size Stack size in 32-bit words.
 * @param pf_task Thread entry function.
 * @param p_arg Thread argument.
 * @return Initial top-of-stack pointer.
 */
uint32_t *akos_port_task_stack_init(uint32_t *p_stack,
                               size_t stack_size,
                               void (*pf_task)(void *),
                               void *p_arg);

#define port_disable_interrupts             { __asm inline("CPSID   I \n"); }
#define port_enable_interrupts              { __asm inline("CPSIE   I \n"); }

/* Make PendSV and SysTick the lowest priority interrupts. */
#define port_setup_PendSV()                 (*(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16))
#define port_trigger_PendSV()               (*(uint32_t volatile *)0xE000ED04 = (1U << 28))

#define port_SVCHandler                     SVC_Handler
#define port_PendSVHandler                  PendSV_Handler
#define port_SysTickHandler                 SysTick_Handler

#ifdef __cplusplus
}
#endif

#endif
