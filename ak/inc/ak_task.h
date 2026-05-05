#ifndef AK_TASK_H
#define AK_TASK_H

#include "ak_list.h"
#include "ak_prio.h"

typedef struct {
  void *sp; /* this must be the first field */
  ak_list_node_t sched_node;
  ak_prio_t prio;
} ak_tcb_t;

#endif /* AK_TASK_H */
