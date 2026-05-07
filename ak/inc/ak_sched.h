#ifndef AK_SCHED_H
#define AK_SCHED_H

#include "ak_task.h"

#include <stdbool.h>

typedef enum { RUNNING, READY, WAITING, BLOCKED } ak_state_t;

extern ak_tcb_t* g_ak_sched_running;
extern ak_tcb_t* g_ak_sched_top_ready;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void ak_sched_reset(void);
bool ak_sched_update_top_ready(void);
bool ak_sched_inc_tick_needs_switch(void);
// void ak_sched_task_switch_state(ak_tcb_t *task, ak_state_t new_state);
void ak_sched_lock(void);
void ak_sched_unlock(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AK_SCHED_H */
