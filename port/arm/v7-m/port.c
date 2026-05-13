#include "port.h"
#include "ak_sched.h"

extern int SysTick_IRQn;
uint8_t g_port_sched_lock_prio;

void port_init(void) {
    g_port_sched_lock_prio = PORT_GET_PRIO(SysTick_IRQn);
}

void SysTick_Handler(void) {
    if (ak_sched_tick()) {
        PORT_SWITCH_CONTEXT();
    }
}

void SVC_Handler(void) {
    __asm__ __volatile__(
        // ICSR.PENDSVSET = 1. Trigger PendSV interrupt
        "ldr    r0, =0xE000ED04             \n\t"
        "ldr    r1, =0x10000000             \n\t"
        "str    r1, [r0]                    \n\t"

        // CONTROL.nPRIV = 1. Thread mode is Unprivileged
        "mrs    r0, control                 \n\t"
        "orr    r0, #1                      \n\t"
        "msr    control, r0                 \n\t"
        "isb                                \n\t"

        // EXC_RETURN to Thread mode using PSP
        "orr    lr, #0xD                    \n\t"
        "bx     lr                          \n\t");
}

void PendSV_Handler(void) {
    __asm__ __volatile__(
        // If g_ak_sched_running == NULL, don't save context
        "ldr    r1, =g_ak_sched_running     \n\t"
        "ldr    r2, [r1]                    \n\t"
        "cbz    r2, PendSV_no_context_save  \n\t"

        // Save current context (R4-R11)
        "mrs    r0, psp                     \n\t"
        "stmdb  r0!, {r4-r11}               \n\t"

        // Save updated SP to running TCB
        "str    r0, [r2]                    \n"

        // g_ak_sched_running = g_ak_sched_top_ready
        "PendSV_no_context_save:            \n\t"
        "ldr    r2, =g_ak_sched_top_ready   \n\t"
        "ldr    r2, [r2]                    \n\t"
        "str    r2, [r1]                    \n\t"

        // Restore new context (R4-R11)
        "ldr    r0, [r2]                    \n\t"
        "ldmia  r0!, {r4-r11}               \n\t"
        "msr    psp, r0                     \n\t"

        // Return to new task
        "bx     lr                          \n\t");
}
