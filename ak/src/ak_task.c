#include "ak_task.h"
#include "ak_mem.h"
#include "ak_sched.h"

ak_tcb_t* ak_task_create(const ak_task_attr_t* attr) {
#ifndef NDEBUG
    if (!attr) {
        return NULL;
    }
#endif  // NDEBUG

    ak_sched_lock();

    size_t alloc_size = AK_ALIGN_CEIL(sizeof(ak_tcb_t) + attr->stack_size);
    ak_tcb_t* tcb = ak_mem_alloc(alloc_size);
    if (tcb) {
        tcb->name = attr->name;
        tcb->prio = attr->prio;
        tcb->state = ZOMBIE;
        tcb->stack_ptr = (uint64_t*)((uint8_t*)tcb + alloc_size) - 1;
        tcb->stack_base = (uint8_t*)tcb + sizeof(ak_tcb_t);
        if (ak_sched_task_switch_state(tcb, READY)) {
#ifdef PORT_SWITCH_CONTEXT
            PORT_SWITCH_CONTEXT();
#endif  // PORT_SWITCH_CONTEXT
        }
    }

    ak_sched_unlock();
    return tcb;
}

int ak_task_delete(ak_tcb_t* task) {
#ifndef NDEBUG
    if (!task) {
        return -1;
    }
#endif  // NDEBUG

    ak_sched_lock();

    if (ak_sched_task_switch_state(g_ak_sched_running, ZOMBIE)) {
        // register idle hook
#ifdef PORT_SWITCH_CONTEXT
        PORT_SWITCH_CONTEXT();
#endif  // PORT_SWITCH_CONTEXT
    }

    ak_sched_unlock();
    return 0;
}

void ak_task_yield(void) {
    ak_sched_lock();
    if (ak_sched_task_switch_state(g_ak_sched_running, READY)) {
#ifdef PORT_SWITCH_CONTEXT
        PORT_SWITCH_CONTEXT();
#endif  // PORT_SWITCH_CONTEXT
    }
    ak_sched_unlock();
}

void ak_task_sleep(uint32_t ticks) {
    ak_sched_lock();
    g_ak_sched_running->sleep_tick = ticks;
    if (ak_sched_task_switch_state(g_ak_sched_running, WAITING)) {
#ifdef PORT_SWITCH_CONTEXT
        PORT_SWITCH_CONTEXT();
#endif  // PORT_SWITCH_CONTEXT
    }
    ak_sched_unlock();
}
