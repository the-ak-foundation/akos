#include <stddef.h>
#include <stdio.h>

#include "ak_cfg.h"
#include "ak_cpu.h"
#include "ak_list.h"
#include "ak_prio.h"
#include "ak_sched.h"
#include "ak_task.h"
#include "port.h"

ak_tcb_t* g_ak_sched_running;
ak_tcb_t* g_ak_sched_top_ready;

static uint32_t _ak_tick;
static uint32_t _ak_sched_lock_nest_cnt;

static ak_tcb_t* _ak_ready_table[AK_CFG_PRIO_MAX + 1];
static ak_tcb_t* _ak_ready_curr[AK_CFG_PRIO_MAX + 1];

static bool _ak_sched_update_top_ready(void);

static int _ak_sched_ready_insert(ak_tcb_t* task);
static int _ak_sched_ready_remove(ak_tcb_t* task);
static int _ak_sched_waiting_insert(ak_tcb_t* task);
static int _ak_sched_waiting_remove(ak_tcb_t* task);
static int _ak_sched_blocked_insert(ak_tcb_t* task);
static int _ak_sched_blocked_remove(ak_tcb_t* task);

void ak_sched_init(void) {
    AK_CPU_CRIT_ENTER();

    g_ak_sched_running = NULL;
    g_ak_sched_top_ready = NULL;
    _ak_tick = 0;
    _ak_sched_lock_nest_cnt = 0;
    ak_prio_reset();
    for (int i = 0; i <= AK_CFG_PRIO_MAX; ++i) {
        _ak_ready_table[i] = _ak_ready_curr[i] = NULL;
    }

    AK_CPU_CRIT_EXIT();
}

void ak_sched_lock(void) {
    AK_CPU_CRIT_ENTER();

    ++_ak_sched_lock_nest_cnt;
#ifdef PORT_SCHED_LOCK
    PORT_SCHED_LOCK();
#endif /* PORT_SCHED_LOCK */

    AK_CPU_CRIT_EXIT();
}

void ak_sched_unlock(void) {
    AK_CPU_CRIT_ENTER();

    --_ak_sched_lock_nest_cnt;
    if (!_ak_sched_lock_nest_cnt) {
#ifdef PORT_SCHED_UNLOCK
        PORT_SCHED_UNLOCK();
#endif /* PORT_SCHED_LOCK */
    }

    AK_CPU_CRIT_EXIT();
}

bool ak_sched_tick(void) {
    AK_CPU_CRIT_ENTER();

    ++_ak_tick;
    // tick increment logic

    AK_CPU_CRIT_EXIT();
}

bool ak_sched_task_switch_state(ak_tcb_t* task, ak_task_state_t new_state) {
    // remove from current list
    // insert to destination list
}

bool _ak_sched_update_top_ready(void) {
    ak_prio_t top_prio = ak_prio_get_top();
    bool has_higher_prio = (top_prio > g_ak_sched_running->prio);
    bool is_prio_single =
        (g_ak_sched_running->sched_node.next == g_ak_sched_running);
    bool res = has_higher_prio || !is_prio_single;
    if (res) {
        g_ak_sched_top_ready = has_higher_prio
                                   ? _ak_ready_curr[top_prio]
                                   : g_ak_sched_running->sched_node.next;
    }
    return res;
}

int _ak_sched_ready_insert(ak_tcb_t* task) {
#ifndef NDEBUG
    if (!task) {
        return -1;
    }
#endif /* NDEBUG */

    int res;

    if (ak_prio_bit_set(task->prio) < 0) {
        res = -1;

    } else {
        res = 0;

        ak_tcb_t** table_entry = &_ak_ready_table[task->prio];
        if (!(*table_entry)) {
            ak_tcb_t** curr_entry = &_ak_ready_curr[task->prio];
            task->sched_node.prev = task->sched_node.next = task;
            *table_entry = *curr_entry = task;
        } else {
            table_entry = ak_list_insert_before(&(*table_entry)->sched_node,
                                                &task->sched_node,
                                                offsetof(ak_tcb_t, sched_node));
        }
    }

    return res;
}

int _ak_sched_ready_remove(ak_tcb_t* task) {
#ifndef NDEBUG
    if (!task) {
        return -1;
    }
#endif /* NDEBUG */

    ak_tcb_t** table_entry = &_ak_ready_table[task->prio];
    ak_tcb_t** curr_entry = &_ak_ready_curr[task->prio];

#ifndef NDEBUG
    if (!(*table_entry)) {
        return -1;
    }
#endif /* NDEBUG */

    ak_tcb_t* next_task =
        ak_list_rm(&task->sched_node, offsetof(ak_tcb_t, sched_node));
    if (*curr_entry == task) {
        *curr_entry = next_task;
    }
    if (*table_entry == task) {
        *table_entry = next_task;
    }
    return 0;
}
