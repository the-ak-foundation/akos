/**
 ******************************************************************************
 * @file    port.h
 * @brief   Cortex-M3 port API for AKOS.
 *
 * @author  Snoopy3921 - AK Foundation
 * @date    Created: 2026-06-11
 * @date    Updated: 2026-06-26
 *
 * @module  AKOS
 ******************************************************************************
 */

#ifndef __PORT_H__
#define __PORT_H__

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Defines -------------------------------------------------------------------*/
// clang-format off

#define PORT_DISABLE_INTERRUPTS             { __asm inline("CPSID   I \n"); } /**< Disable global interrupts. */
#define PORT_ENABLE_INTERRUPTS              { __asm inline("CPSIE   I \n"); } /**< Enable global interrupts. */

/* Make PendSV and SysTick the lowest priority interrupts. */
#define PORT_SETUP_PENDSV()                 (*(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16)) /**< Configure PendSV priority. */
#define PORT_TRIGGER_PENDSV()               (*(uint32_t volatile *)0xE000ED04 = (1U << 28))     /**< Trigger PendSV exception. */

#define PORT_SVC_HANDLER(void)               SVC_Handler(void)     /**< Map AKOS SVC handler to CMSIS name. */
#define PORT_PENDSV_HANDLER(void)            PendSV_Handler(void)  /**< Map AKOS PendSV handler to CMSIS name. */
#define PORT_SYSTICK_HANDLER(void)           SysTick_Handler(void) /**< Map AKOS SysTick handler to CMSIS name. */

// clang-format on

/* Function prototypes -------------------------------------------------------*/
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
uint32_t* akos_port_task_stack_init(uint32_t* p_stack, size_t stack_size, void (*pf_task)(void*),
                                    void* p_arg);

#ifdef __cplusplus
}
#endif

#endif /* __PORT_H__ */
