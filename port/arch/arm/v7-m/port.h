#ifndef ARM_V7M_PORT_H
#define ARM_V7M_PORT_H

#include <stdint.h>

#include "cmsis_compiler.h"

#define PORT_EN_IRQ()             __enable_irq()
#define PORT_DIS_IRQ()            __disable_irq()
#define PORT_GET_IRQ_STATE()      __get_PRIMASK()
#define PORT_SET_IRQ_STATE(state) __set_PRIMASK(state)
#define PORT_GET_PRIO(irq)        __NVIC_GetPriority(irq)
#define PORT_SET_PRIO(irq)        __NVIC_SetPriority(irq)
#define PORT_SCHED_LOCK()         __set_BASEPRI(g_port_sched_lock_prio)
#define PORT_SCHED_UNLOCK()       __set_BASEPRI(0)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern uint8_t g_port_sched_lock_prio;

extern uint32_t __NVIC_GetPriority(int irq);
extern void __NVIC_SetPriority(int irq, uint32_t prio);

void port_init(void);
void PendSV_Handler(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ARM_V7M_PORT_H */
