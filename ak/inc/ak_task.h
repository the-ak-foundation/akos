#ifndef AK_TASK_H
#define AK_TASK_H

#include "ak_list.h"
#include "ak_prio.h"

#include <stddef.h>

typedef enum { RUNNING, READY, WAITING, BLOCKED, ZOMBIE } ak_task_state_t;

typedef struct {
    void* stack_ptr;  // this must be the first field
    void* stack_base;

    const char* name;
    ak_prio_t prio;
    ak_task_state_t state;
    uint32_t sleep_tick;
    ak_list_node_t sched_node;
} ak_tcb_t;

typedef struct {
    const char* name;
    size_t stack_size;
    ak_prio_t prio;
} ak_task_attr_t;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

ak_tcb_t* ak_task_create(const ak_task_attr_t* attr);
int ak_task_delete(ak_tcb_t* task);
void ak_task_yield(void);
void ak_task_sleep(uint32_t ticks);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // AK_TASK_H
