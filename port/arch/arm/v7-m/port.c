#include "port.h"

extern int SysTick_IRQn;
uint8_t g_port_sched_lock_prio;

void port_init(void) { g_port_sched_lock_prio = PORT_GET_PRIO(SysTick_IRQn); }

void PendSV_Handler(void) {
  __asm__ __volatile__(
    "push {r4-r11}\n"
    ""
  );
}
