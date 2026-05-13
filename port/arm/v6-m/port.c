#include "port.h"

void SVC_Handler(void) {
    __asm__ __volatile__(
        // r0 = g_ak_sched_running->sp
        "ldr    r0, =g_ak_sched_running     \n\t"
        "ldr    r0, [r0]                    \n\t"
        "ldr    r0, [r0]                    \n\t"

        // Restore R8-R11 (High registers)
        "adds   r0, #16                     \n\t"
        "ldmia  r0!, {r4-r7}                \n\t"
        "mov    r8, r4                      \n\t"
        "mov    r9, r5                      \n\t"
        "mov    r10, r6                     \n\t"
        "mov    r11, r7                     \n\t"

        // Update PSP to the task's stack
        "msr    psp, r0                     \n\t"

        // Restore R4-R7 (Low registers)
        "subs   r0, #32                     \n\t"
        "ldmia  r0!, {r4-r7}                \n\t"

        // Set EXC_RETURN to 0xFFFFFFFD (Thread mode, PSP)
        "mov    r0, #0xD                    \n\t"
        "orrs   lr, r0                      \n\t"
        "bx     lr                          \n\t");
}

void PendSV_Handler(void) {
    __asm__ __volatile__(
        // Prepare stack for callee-save registers
        "mrs    r0, psp                     \n\t"
        "subs   r0, #32                     \n\t"

        // Save SP to g_ak_sched_running->sp
        "ldr    r1, =g_ak_sched_running     \n\t"
        "ldr    r2, [r1]                    \n\t"
        "str    r0, [r2]                    \n\t"

        // Save R4-R7 and R8-R11 (Double-hop for v6-M)
        "stmia  r0!, {r4-r7}                \n\t"
        "mov    r4, r8                      \n\t"
        "mov    r5, r9                      \n\t"
        "mov    r6, r10                     \n\t"
        "mov    r7, r11                     \n\t"
        "stmia  r0!, {r4-r7}                \n\t"

        // g_ak_sched_running = g_ak_sched_top_ready
        "ldr    r2, =g_ak_sched_top_ready   \n\t"
        "ldr    r2, [r2]                    \n\t"
        "str    r2, [r1]                    \n\t"

        // Restore R8-R11 via high-register move
        "ldr    r0, [r2]                    \n\t"
        "adds   r0, #16                     \n\t"
        "ldmia  r0!, {r4-r7}                \n\t"
        "mov    r8, r4                      \n\t"
        "mov    r9, r5                      \n\t"
        "mov    r10, r6                     \n\t"
        "mov    r11, r7                     \n\t"

        // Update PSP to the new task stack
        "msr    psp, r0                     \n\t"

        // Restore R4-R7 from the base of the new frame
        "subs   r0, #32                     \n\t"
        "ldmia  r0!, {r4-r7}                \n\t"

        "bx     lr                          \n\t");
}
