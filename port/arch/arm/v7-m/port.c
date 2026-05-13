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
        // load the first task's stack pointer
        "ldr    r0, g_ak_sched_running;"
        "ldr    r0, [r0];"
        "ldmia  r0!, {r4-r11};"
        "msr    psp, r0;"

        // CONTROL.nPRIV=1. Thread has unprivileged access.
        "mrs    r1, control;"
        "orr    r1, #1;"
        "msr    control, r1;"
        "isb;"

        // use process stack pointer (PSP) after exception return
        "orr    lr, #0xD;"
        "bx     lr;");
}

void PendSV_Handler(void) {
    __asm__ __volatile__(
        // push callee-save registers
        "mrs    r0, psp;"
        "stmdb  r0!, {r4-r11};"

        // save SP to running TCB
        "ldr    r1, =g_ak_sched_running;"
        "ldr    r2, [r1];"
        "str    r0, [r2];"

        // set running TCB to top ready TCB
        "ldr    r2, g_ak_sched_top_ready;"
        "str    r2, [r1];"

        // pop callee-save registers
        "ldr    r0, [r2];"
        "ldmia  r0!, {r4-r11};"
        "msr    psp, r0;"

        "bx     lr;");
}
