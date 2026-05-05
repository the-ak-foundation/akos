#include "ak_sched.h"
#include "ak_cfg.h"
#include "ak_cpu.h"
#include "ak_list.h"
#include "ak_prio.h"
#include "ak_task.h"
#include "port.h"

#include <stddef.h>
#include <stdio.h>

ak_tcb_t* g_ak_sched_run;

static uint32_t _ak_sched_lock_nest_cnt;
static ak_tcb_t* _ak_rdy_tbl[AK_CFG_PRIO_MAX + 1];
static ak_tcb_t* _ak_rdy_runs[AK_CFG_PRIO_MAX + 1];

static int _ak_sched_rdy_ins(ak_tcb_t* task);
static int _ak_sched_rdy_rm(ak_tcb_t* task);

void ak_sched_reset(void) {
  AK_CPU_CRIT_ENTER();

  g_ak_sched_run = NULL;
  g_ak_sched_high_rdy = NULL;
  ak_prio_reset();
  for (int i = 0; i <= AK_CFG_PRIO_MAX; ++i) {
    _ak_rdy_tbl[i] = _ak_rdy_runs[i] = NULL;
  }

  AK_CPU_CRIT_EXIT();
}

bool ak_sched_rotate(void) {
  AK_CPU_CRIT_ENTER();

  ak_prio_t top_prio = ak_prio_get_top();
  bool has_higher_prio = (top_prio > g_ak_sched_run->prio);
  bool is_prio_single = (g_ak_sched_run->sched_node.next == g_ak_sched_run);
  bool res = has_higher_prio || !is_prio_single;
  if (res) {
    g_ak_sched_high_rdy = has_higher_prio ? _ak_rdy_runs[top_prio]
                                          : g_ak_sched_run->sched_node.next;
  }

  AK_CPU_CRIT_EXIT();
  return res;
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
#ifdef PORT_SCHED_LOCK
    PORT_SCHED_UNLOCK();
#endif /* PORT_SCHED_LOCK */
  }

  AK_CPU_CRIT_EXIT();
}

int _ak_sched_rdy_ins(ak_tcb_t* task) {
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

    ak_tcb_t** tbl_ent = &_ak_rdy_tbl[task->prio];
    if (!(*tbl_ent)) {
      ak_tcb_t** runs_ent = &_ak_rdy_runs[task->prio];
      task->sched_node.prev = task->sched_node.next = task;
      *tbl_ent = *runs_ent = task;
    } else {
      tbl_ent = ak_list_ins_bef(
        &(*tbl_ent)->sched_node, &task->sched_node,
        offsetof(ak_tcb_t, sched_node)
      );
    }
  }

  return res;
}

int _ak_sched_rdy_rm(ak_tcb_t* task) {
#ifndef NDEBUG
  if (!task) {
    return -1;
  }
#endif /* NDEBUG */

  ak_tcb_t** tbl_ent = &_ak_rdy_tbl[task->prio];
  ak_tcb_t** runs_ent = &_ak_rdy_runs[task->prio];

#ifndef NDEBUG
  if (!(*tbl_ent)) {
    return -1;
  }
#endif /* NDEBUG */

  ak_tcb_t* next_task =
    ak_list_rm(&task->sched_node, offsetof(ak_tcb_t, sched_node));
  if (*runs_ent == task) {
    *runs_ent = next_task;
  }
  if (*tbl_ent == task) {
    *tbl_ent = next_task;
  }
  return 0;
}
