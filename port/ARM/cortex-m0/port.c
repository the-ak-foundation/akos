/**
  ******************************************************************************
  * @file    port.c
  * @brief   Cortex-M0 port implementation, context switching and tick hook.
  *
  * @author  Snoopy3921 - AK Foundation
  * @date    Created: 2026-06-11
  * @date    Updated: 2026-06-26
  *
  * @module  AKOS
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "port.h"
#include "config.h"
#include "core.h"
#ifdef AKOS_PORT_DEVICE_HEADER
#include AKOS_PORT_DEVICE_HEADER
#else
#include "core_cm0.h"
#include "core_cmFunc.h"
#endif
#include "thread.h"

/* Function definitions ------------------------------------------------------*/
/**
 * @brief Initialize SysTick to generate 1ms OS ticks.
 * @param cpu_freq Core clock frequency in Hz.
 */
void akos_port_systick_init_freq(uint32_t cpu_freq)
{
    volatile uint32_t ticks = cpu_freq / 1000u;

    SysTick->LOAD = ticks - 1u;
    SysTick->VAL  = 0u;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;

    *(volatile uint32_t *)0xE000ED20UL &= ~(0xFFUL << 24);
    *(volatile uint32_t *)0xE000ED20UL |= ((1 << __NVIC_PRIO_BITS) - 2)
                                          << (32 - __NVIC_PRIO_BITS);
}

/**
 * @brief Start the first thread by restoring the initial exception context.
 */
void akos_port_start_first_task(void)
{
    __asm volatile(
        " ldr r0, =0xE000ED08       \n" /* Use the NVIC offset register to
                                           locate the stack. */
        " ldr r0, [r0]              \n"
        " ldr r0, [r0]              \n"
        " msr msp, r0               \n" /* Set the msp back to the start of the
                                           stack. */
        " cpsie i                   \n" /* Globally enable interrupts. */
        " cpsie f                   \n"
        " dsb                       \n"
        " isb                       \n"
        " svc 0                     \n" /* System call to start first thread. */
        " nop                       \n"
        " .ltorg                    \n");
}

/**
 * @brief Build the initial Cortex-M thread stack frame.
 * @param p_stack Base stack buffer.
 * @param stack_size Stack size in 32-bit words.
 * @param pf_task Thread entry function.
 * @param p_arg Task argument.
 * @return Initial top-of-stack pointer.
 */
uint32_t *akos_port_task_stack_init(uint32_t *p_stack,
                                    size_t stack_size,
                                    void (*pf_task)(void *),
                                    void *p_arg)
{
    uint32_t *p_stack_ptr;

    p_stack_ptr = &p_stack[stack_size - (uint32_t)1];
    p_stack_ptr = (uint32_t *)(((uint32_t)p_stack_ptr) & (~((uint32_t)0x007)));

    *(--p_stack_ptr) = (uint32_t)0x01000000UL;
    *(--p_stack_ptr) = ((uint32_t)pf_task) & ((uint32_t)0xfffffffeUL);
    *(--p_stack_ptr) = (uint32_t)0x0000000EUL;
    p_stack_ptr -= 5;
    *p_stack_ptr = (uint32_t)p_arg;
    p_stack_ptr -= 8;

    return p_stack_ptr;
}

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @brief SVC handler used to restore the first thread context.
 */
void port_SVCHandler(void)
{
    __asm volatile(
        ".syntax unified                \n"
        "cpsid   i                      \n"   // Prevent interrupt during
                                              // context switch
        "ldr     r1, =tcb_curr_ptr      \n"   // get pointer to TCB current
        "ldr     r1, [r1]               \n"   // get TCB current = pointer to
                                              // StkPtr
        "ldr     r0, [r1]               \n"   // get StkPtr

        "adds    r0, #16                \n"
        "ldmia   r0!, {r4-r7}           \n"   //
        "msr     psp, r0                \n"   //

        // Double-hop context restore
        "mov     r8, r4                 \n"
        "mov     r9, r5                 \n"
        "mov     r10, r6                \n"
        "mov     r11, r7                \n"
        "subs    r0, #32                \n"
        "ldmia   r0!, {r4-r7}           \n"

        "mov     r0, lr                 \n"
        "movs    r1, #0xD               \n"
        "orrs    r0, r1                 \n"   // LR = 0xFFFFFFFD return to
                                              // threadmode
        "mov     lr, r0                 \n"
        "cpsie   i                      \n"   //
        "bx      lr                     \n"   //

        ".ltorg                         \n");
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @brief PendSV handler used for context switching between threads.
 */
void port_PendSVHandler(void)
{
    __asm volatile(
        ".syntax unified                            \n"
        "mrs     r0, psp                            \n"   // PSP is process
                                                          // stack pointer
        "cmp     r0, #0                             \n"
        "beq     OS_CPU_PendSVHandler_nosave        \n"

        "subs    r0, #32                            \n"   // Save remaining regs
                                                          // r4-11 on process
                                                          // stack
        "stmia   r0!, {r4-r7}                       \n"   //
        "mov     r8, r4                             \n"
        "mov     r9, r5                             \n"
        "mov     r10, r6                            \n"
        "mov     r11, r7                            \n"
        "stmia   r0!, {r4-r7}                       \n"
        "subs    r0, #32                            \n"

        "ldr     r1, =tcb_curr_ptr                  \n"   // OSTCBCur->OSTCBStkPtr
                                                          // = SP;
        "ldr     r1, [r1]                           \n"   //
        "str     r0, [r1]                           \n"   // R0 is SP of process
                                                          // being switched out

        /* At this point, entire context of process has been saved    */

        "OS_CPU_PendSVHandler_nosave:               \n"   //
        "ldr     r0, =tcb_curr_ptr                  \n"   // OSTCBCur  =
                                                          // OSTCBHighRdy;
        "ldr     r1, =tcb_high_rdy_ptr              \n"   //
        "ldr     r2, [r1]                           \n"   //
        "str     r2, [r0]                           \n"   //

        "ldr     r0, [r2]                           \n"   // R0 is new process
                                                          // SP; SP =
                                                          // OSTCBHighRdy->OSTCBStkPtr;
        "adds    r0, #16                            \n"
        "ldmia   r0!, {r4-r7}                       \n"   // Restore r4-11 from
                                                          // new process stack

        "msr     psp, r0                            \n"   // Load PSP with new
                                                          // process SP

        "mov     r8, r4                             \n"
        "mov     r9, r5                             \n"
        "mov     r10, r6                            \n"
        "mov     r11, r7                            \n"
        "subs    r0, #32                            \n"   //
        "ldmia   r0!, {r4-r7}                       \n"

        "mov     r0, lr                             \n"
        "movs    r1, #0xD                           \n"
        "orrs    r0, r1                             \n"   // Ensure exception
                                                          // return uses process
                                                          // stack
        "mov     lr, r0                             \n"
        "bx      lr                                 \n"   // Exception return
                                                          // will restore
                                                          // remaining context

        ".ltorg                                     \n");
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @brief SysTick ISR hook for scheduler tick updates.
 */
void port_SysTickHandler(void)
{
    port_disable_interrupts
    /* Increment the RTOS tick. */
    if (akos_thread_increment_tick() == OS_TRUE)
    {
        /* A context switch is required.  Context switching is performed in
         * the PendSV interrupt.  Pend the PendSV interrupt. */
        port_trigger_PendSV();
    }
    port_enable_interrupts
}

#ifdef __cplusplus
}
#endif
