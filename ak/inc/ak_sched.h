#ifndef AK_SCHED_H
#define AK_SCHED_H

#include "ak_task.h"

#include <stdbool.h>

extern ak_tcb_t* g_ak_sched_running;
extern ak_tcb_t* g_ak_sched_top_ready;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void ak_sched_init(void);
void ak_sched_lock(void);
void ak_sched_unlock(void);

bool ak_sched_tick(void);
bool ak_sched_task_switch_state(ak_tcb_t* task, ak_task_state_t new_state);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // AK_SCHED_H
