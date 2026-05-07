/**
 * @file thread.c
 * @brief Thread management, scheduling, and thread messaging integration.
 */

#include "thread.h"
#include "core.h"
#include "list.h"
#include "memory.h"
#include "timer.h"
#include "priority.h"
#include "port.h"
#include <string.h>

#define TASK_IDLE_PRI               (OS_CFG_PRIO_MAX - 1u)

#define TASK_TIMER_PRI              ((uint8_t)OS_CFG_TIMER_TASK_PRI) 

#define TASK_TIMER_STK_SIZE         (100u) 


typedef struct task_tcb task_tcb_t;

#define SIZE_OF_TCB                 (sizeof(task_tcb_t))


task_tcb_t *volatile tcb_curr_ptr = NULL;
task_tcb_t *volatile tcb_high_rdy_ptr = NULL;

static list_t rdy_task_list[OS_CFG_PRIO_MAX];       /*< Prioritised ready tasks. */
static list_t dly_task_list_1;                      /*< Delayed tasks. */
static list_t dly_task_list_2;                      /*< Delayed tasks (two lists are used - one for delays that have overflowed the current tick count. */
static list_t *volatile dly_task_list_ptr;          /*< Points to the delayed task list currently being used. */
static list_t *volatile overflow_dly_task_list_ptr; /*< Points to the delayed task list currently being used to hold tasks that have overflowed the current tick count. */
static list_t suspended_task_list;                  /*< Tasks that are currently suspended. */

static volatile uint16_t num_of_tasks           = (uint16_t)0U;
static volatile uint32_t tick_count             = (uint32_t)0u;
static volatile uint32_t ticks_pended           = (uint32_t)0U;
static volatile uint32_t next_tick_to_unblock   = (uint32_t)OS_CFG_DELAY_MAX; /* Initialised to portMAX_DELAY before the scheduler starts. */

static volatile uint8_t sched_is_running        = (uint8_t)OS_FALSE;

extern const thread_t __start_task_desc[] __attribute__((weak));
extern const thread_t __stop_task_desc[] __attribute__((weak));

static task_tcb_t **task_tcb_list = NULL; /*< Holds the list of task TCBs. */
static uint8_t task_tcb_list_len = 0u;
static uint8_t task_app_count = 0u;
static uint8_t task_idle_id = 0u;
static uint8_t task_timer_id = 0u;

/**
 * @brief Get current tick counter.
 * @return Current tick value.
 */
uint32_t akos_thread_get_tick(void)
{
    return tick_count;
}

uint8_t akos_thread_get_app_thread_count(void)
{
    return task_app_count;
}

uint8_t akos_thread_get_idle_thread_id(void)
{
    return task_idle_id;
}

uint8_t akos_thread_get_timer_thread_id(void)
{
    return task_timer_id;
}

/**
 * @brief Idle task entry function.
 * @param p_arg Unused argument.
 */
static void task_idle_func(void *p_arg)
{
    for (;;)
    {
    }
}

/**
 * @brief Internal timer task entry function.
 * @param p_arg Unused argument.
 */
static void task_timer_func(void *p_arg)
{
    for (;;)
    {
        akos_timer_processing();
    }
}

struct task_tcb
{
    volatile uint32_t *stk_ptr;  /* Stack pointer, has to be the first member of TCB        */
    list_item_t state_list_item; /*Item in StateList include Ready, Blocked, Suspended List */
    list_item_t event_list_item; /*Item in Event List */
    uint32_t *stk_limit_ptr;     /* Pointer used to set stack 'watermark' limit             */
    uint8_t prio;
    size_t stk_size;            /* Size of task stack (in number of stack elements)         */
    uint32_t *stk_base_ptr;     /* Pointer to base address of stack 					    */
    thread_id_t id;
    msg_queue_t msg_queue;
    thread_state_t state;         /* States */
};

/**
 * @brief Initialize all scheduler lists.
 */
static void init_task_lists(void)
{
    /* Initialize lists */
    uint8_t prio;
    for (prio = 0; prio < OS_CFG_PRIO_MAX; prio++)
    {
        akos_list_init(&(rdy_task_list[prio]));
    }
    akos_list_init(&dly_task_list_1);
    akos_list_init(&dly_task_list_2);
    akos_list_init(&suspended_task_list);
    dly_task_list_ptr = &dly_task_list_1;
    overflow_dly_task_list_ptr = &dly_task_list_2;
    /********************/
}

/**
 * @brief Swap active and overflow delay lists on tick wrap.
 */
static void task_switch_delay_lists()
{
    list_t *p_list_temp;
    p_list_temp = dly_task_list_ptr;
    dly_task_list_ptr = overflow_dly_task_list_ptr;
    overflow_dly_task_list_ptr = p_list_temp;

    if (list_is_empty(dly_task_list_ptr) == OS_TRUE)
    {
        /* The new current delayed list is empty.  Set xNextTaskUnblockTime to
         * the maximum possible value so it is  extremely unlikely that the
         * if( xTickCount >= xNextTaskUnblockTime ) test will pass until
         * there is an item in the delayed list. */
        next_tick_to_unblock = OS_CFG_DELAY_MAX;
    }
    else
    {
        /* The new current delayed list is not empty, get the value of
         * the item at the head of the delayed list.  This is the time at
         * which the task at the head of the delayed list should be removed
         * from the Blocked state. */
        task_tcb_t *p_tcb = list_get_owner_of_head_item(dly_task_list_ptr);
        uint32_t item_value = list_item_get_value(&(p_tcb->state_list_item));
        next_tick_to_unblock = item_value;
    }
}

/**
 * @brief Add new task to ready list and update scheduler state.
 * @param p_tcb Task control block.
 */
static void add_new_task_to_rdy_list(task_tcb_t *p_tcb)
{
    AKOS_CORE_ENTER_CRITICAL();
    {
        num_of_tasks++;
        if (num_of_tasks == 1)
        {
            init_task_lists();
            tcb_curr_ptr = p_tcb;
        }
        else
        {
            if (tcb_curr_ptr->prio <= p_tcb->prio)
            {
                tcb_curr_ptr = p_tcb;
            }
        }
        akos_list_insert_end(&(rdy_task_list[p_tcb->prio]), &((p_tcb)->state_list_item));
        if (list_get_num_item(&(rdy_task_list[p_tcb->prio])) == 1u)
        {
            akos_priority_insert(p_tcb->prio);
        }
        /*Save state*/
        p_tcb->state = THREAD_STATE_READY;
    }
    AKOS_CORE_EXIT_CRITICAL();
}

/**
 * @brief Add task to corresponding ready queue.
 * @param p_tcb Task control block.
 */
static void add_task_to_rdy_list(task_tcb_t *p_tcb)
{
    akos_list_insert_end(&(rdy_task_list[p_tcb->prio]), &((p_tcb)->state_list_item));
    if (list_get_num_item(&(rdy_task_list[p_tcb->prio])) == 1u)
    {
        akos_priority_insert(p_tcb->prio);
    }
    /*Save state*/
    p_tcb->state = THREAD_STATE_READY;
}

/**
 * @brief Move current task to delay/suspend lists.
 * @param tick_to_delay Delay ticks.
 * @param can_block_indefinitely Whether infinite block is allowed.
 */
static void add_curr_task_to_delay_list(uint32_t tick_to_delay, uint8_t can_block_indefinitely)
{
    uint32_t time_to_wake;
    const uint32_t const_tick = tick_count;
    if (akos_list_remove(&(tcb_curr_ptr->state_list_item)) == 0u)
    {
        akos_priority_remove(tcb_curr_ptr->prio);
    }
    if ((tick_to_delay == OS_CFG_DELAY_MAX) && (can_block_indefinitely != OS_FALSE))
    {
        /* Add the task to the suspended task list instead of a delayed task
         * list to ensure it is not woken by a timing event.  It will block
         * indefinitely. */
        akos_list_insert_end(&suspended_task_list, &(tcb_curr_ptr->state_list_item));
        /*Save state*/
        tcb_curr_ptr->state = THREAD_STATE_SUSPENDED;
    }
    else
    {
        time_to_wake = const_tick + tick_to_delay;
        list_item_set_value(&(tcb_curr_ptr->state_list_item), time_to_wake);

        if (time_to_wake < const_tick)
        {
            /* Wake time has overflowed.  Place this item in the overflow
             * list. */
            akos_list_insert(overflow_dly_task_list_ptr, &(tcb_curr_ptr->state_list_item));
        }
        else
        {
            /* The wake time has not overflowed, so the current block list
             * is used. */
            akos_list_insert(dly_task_list_ptr, &(tcb_curr_ptr->state_list_item));

            /*Update next tick to block is important in order scheduler not to miss this stamp
            Just update next tick to block in this branch because overflow delay is just the background list
            */

            if (time_to_wake < next_tick_to_unblock)
            {
                next_tick_to_unblock = time_to_wake;
            }
        }
        /*Save state*/
        tcb_curr_ptr->state = THREAD_STATE_DELAYED;
    }
    uint8_t highest_prio = akos_priority_get_highest();
    tcb_high_rdy_ptr = list_get_owner_of_head_item(&(rdy_task_list[highest_prio]));

    /*Save state*/
    tcb_high_rdy_ptr->state = THREAD_STATE_RUNNING;
}

/**
 * @brief Internal task creation helper.
 * @param id Task ID.
 * @param pf_task Task entry function.
 * @param p_arg Task argument.
 * @param prio Task priority.
 * @param queue_size Message queue size.
 * @param stack_size Stack size in words.
 * @return Created TCB pointer, or NULL on failure.
 */
static task_tcb_t *task_create(thread_id_t id,
                               thread_func_t pf_thread,
                               void *p_arg,
                               uint8_t prio,
                               size_t queue_size,
                               size_t stack_size)
{
    if (sched_is_running == OS_TRUE)
    {
        // OSUniversalError = OS_ERR_SCHED_IS_RUNNING;
        core_assert(0, "OS_ERR_SCHED_IS_RUNNING");
        return NULL;
    }
    if (prio > (OS_CFG_PRIO_MAX - 1U))
    {
        // OSUniversalError = OS_ERR_TCB_PRIO_INVALID;
        core_assert(0, "OS_ERR_TCB_PRIO_INVALID");
        return NULL;
    }
    if (pf_thread == NULL)
    {
        // OSUniversalError = OS_ERR_TCB_FUNC_INVALID;
        core_assert(0, "OS_ERR_TCB_FUNC_INVALID");
        return NULL;
    }
    if (stack_size < OS_CFG_TASK_STK_SIZE_MIN)
    {
        // OSUniversalError = OS_ERR_TCB_STK_SIZE_INVALID;
        core_assert(0, "OS_ERR_TCB_STK_SIZE_INVALID");
        return NULL;
    }
    if ((task_tcb_list == NULL) || (id >= task_tcb_list_len))
    {
        core_assert(0, "OS_ERR_TASK_ID_INVALID");
        return NULL;
    }
    if (task_tcb_list[id] != NULL)
    {
        core_assert(0, "OS_ERR_TASK_ID_ALREADY_USED");
        return NULL;
    }
    

    task_tcb_t *p_new_tcb;
    uint32_t *p_stack;

    p_stack = akos_memory_malloc(stack_size * sizeof(uint32_t));
    if (p_stack != NULL)
    {
        p_new_tcb = (task_tcb_t *)akos_memory_malloc(SIZE_OF_TCB);
        if (p_new_tcb != NULL)
        {
            memset((void *)p_new_tcb, 0x00, SIZE_OF_TCB);

            /*Save stack limit pointer*/
            p_new_tcb->stk_limit_ptr = p_stack;
        }
        else
        {
            akos_memory_free(p_stack);
            // OSUniversalError = OS_ERR_TCB_NOT_ENOUGH_MEM_ALLOC;
            core_assert(0, "OS_ERR_TCB_NOT_ENOUGH_MEM_ALLOC");
            return NULL;
        }
    }
    else
    {
        // OSUniversalError = OS_ERR_TCB_NOT_ENOUGH_MEM_ALLOC;
        core_assert(0, "OS_ERR_TCB_NOT_ENOUGH_MEM_ALLOC");
        return NULL;
    }

    /*Now TCB and stack are created*/
    uint32_t *p_stack_ptr;

    /* Fill the stack with a known value to assist debugging. */
    //( void ) memset( p_new_tcb->stk_limit_ptr, OS_CFG_TASK_STACK_FILL_BYTE, stack_size );

    /*Init stack frame*/
    p_stack_ptr = akos_port_task_stack_init(p_stack,
                                       stack_size,
                                       pf_thread,
                                       p_arg);

    /*Save top of stack (Stack pointer)*/
    p_new_tcb->stk_ptr = p_stack_ptr;

    /*Save stack size*/
    p_new_tcb->stk_size = stack_size;

    /*Save ID*/
    p_new_tcb->id = id;

    /*Save prio*/
    p_new_tcb->prio = prio;
    // akos_priority_insert(prio);

    akos_message_queue_init(&(p_new_tcb->msg_queue), queue_size);

    /* Init linked lists */
    akos_list_item_init(&(p_new_tcb->state_list_item));
    akos_list_item_init(&(p_new_tcb->event_list_item));

    list_item_set_owner(&(p_new_tcb->state_list_item), (void *)p_new_tcb);
    list_item_set_owner(&(p_new_tcb->event_list_item), (void *)p_new_tcb);

    list_item_set_value(&(p_new_tcb->state_list_item), prio);

    add_new_task_to_rdy_list(p_new_tcb);
    task_tcb_list[id] = p_new_tcb;

    return p_new_tcb;
}

/**
 * @brief Create all statically defined application tasks plus system tasks.
 */
void akos_thread_register_static_threads(void)
{
    const thread_t *p_thread_desc = __start_task_desc;
    const thread_t *p_thread_desc_end = __stop_task_desc;
    size_t app_count = 0u;

    if ((p_thread_desc == NULL) || (p_thread_desc_end == NULL) || (p_thread_desc == p_thread_desc_end))
    {
        /* No application threads were linked in. */
    }
    else
    {
        app_count = (size_t)(p_thread_desc_end - p_thread_desc);
    }

    if (app_count > (size_t)(UINT8_MAX - 2u))
    {
        core_assert(0, "OS_ERR_TOO_MANY_TASKS");
        return;
    }

    task_app_count = (uint8_t)app_count;
    task_idle_id = task_app_count;
    task_timer_id = (uint8_t)(task_app_count + 1u);
    task_tcb_list_len = (uint8_t)(task_app_count + 2u);

    task_tcb_list = (task_tcb_t **)akos_memory_malloc(sizeof(task_tcb_t *) * task_tcb_list_len);
    if (task_tcb_list == NULL)
    {
        core_assert(0, "OS_ERR_TASK_TCB_TABLE_ALLOC");
        return;
    }
    memset(task_tcb_list, 0x00, sizeof(task_tcb_t *) * task_tcb_list_len);

    for (; p_thread_desc < p_thread_desc_end; ++p_thread_desc)
    {
        if (p_thread_desc->id >= task_app_count)
        {
            core_assert(0, "OS_ERR_THREAD_ID_RESERVED");
            continue;
        }

        (void)task_create(p_thread_desc->id,
                          p_thread_desc->pf_thread,
                          p_thread_desc->p_arg,
                          p_thread_desc->prio,
                          p_thread_desc->queue_size,
                          p_thread_desc->stack_size);
    }

    (void)task_create((thread_id_t)task_timer_id,
                      (thread_func_t)task_timer_func,
                      (void *)NULL,
                      (uint8_t)TASK_TIMER_PRI,
                      (size_t)(OS_CFG_TASK_MSG_Q_SIZE_NORMAL),
                      (size_t)TASK_TIMER_STK_SIZE);

    (void)task_create((thread_id_t)task_idle_id,
                      (thread_func_t)task_idle_func,
                      (void *)NULL,
                      (uint8_t)TASK_IDLE_PRI,
                      (size_t)(0u),
                      (size_t)OS_CFG_TASK_STK_SIZE_MIN);
}

/**
 * @brief Tick hook: unblock delayed tasks and select next runnable task.
 * @return OS_TRUE if context switch is needed, otherwise OS_FALSE.
 */
uint8_t akos_thread_increment_tick(void)
{
    uint8_t is_switch_needed = OS_FALSE;
    task_tcb_t *p_tcb;
    uint32_t item_value;

    const uint32_t const_tick = tick_count + (uint32_t)1;

    /* Increment the RTOS tick, switching the delayed and overflowed
     * delayed lists if it wraps to 0. */
    tick_count = const_tick;

    if (const_tick == (uint32_t)0U) /* Overflowed, switch delaylist*/
    {
        task_switch_delay_lists();
    }

    if (const_tick >= next_tick_to_unblock)
    {
        for (;;)
        {
            if (list_is_empty(dly_task_list_ptr) == OS_TRUE)
            {
                next_tick_to_unblock = OS_CFG_DELAY_MAX;
                break;
            }
            else
            {
                p_tcb = list_get_owner_of_head_item(dly_task_list_ptr);
                item_value = list_item_get_value(&(p_tcb->state_list_item));
                if (item_value > const_tick)
                {
                    /* Stop condition */
                    next_tick_to_unblock = item_value;
                    break;
                }
                akos_list_remove(&(p_tcb->state_list_item)); /*Remove from block state*/

                /* Is the task waiting on an event also?  If so remove
                 * it from the event list. */
                if (list_item_get_list_contain(&(p_tcb->event_list_item)) != NULL)
                {
                    akos_list_remove(&(p_tcb->event_list_item));
                }
                add_task_to_rdy_list(p_tcb);
                if (p_tcb->prio < tcb_curr_ptr->prio)
                {
                    tcb_high_rdy_ptr = p_tcb;
                    is_switch_needed = OS_TRUE;
                }
            }
        }
    }

    uint8_t highest_prio = akos_priority_get_highest();
    if (list_get_num_item(&(rdy_task_list[highest_prio])) > 1u)
    {
        tcb_high_rdy_ptr = akos_list_get_owner_of_next_item(&(rdy_task_list[highest_prio]));
        is_switch_needed = OS_TRUE;
    }
    /*Save state*/
    if(is_switch_needed == OS_TRUE) tcb_high_rdy_ptr->state = THREAD_STATE_RUNNING;
    
    return is_switch_needed;
}

/**
 * @brief Delay current thread for a number of ticks.
 * @param tick_to_delay Delay in ticks.
 */
void akos_thread_delay(const uint32_t tick_to_delay)
{
    if (tick_to_delay > (uint32_t)0U)
    {
        AKOS_CORE_ENTER_CRITICAL();
        add_curr_task_to_delay_list(tick_to_delay, OS_FALSE);
        port_trigger_PendSV();
        AKOS_CORE_EXIT_CRITICAL();
    }
}

/**
 * @brief Start scheduler state variables.
 */
void akos_thread_start(void)
{
    tcb_high_rdy_ptr = list_get_owner_of_head_item(&(rdy_task_list[akos_priority_get_highest()]));
    tcb_curr_ptr = tcb_high_rdy_ptr;
    tick_count = 0u;
    next_tick_to_unblock = OS_CFG_DELAY_MAX;
    sched_is_running = OS_TRUE;
}

/**
 * @brief Post dynamic payload message to another thread.
 * @param des_thread_id Destination thread ID.
 * @param sig Message signal.
 * @param p_content Payload pointer.
 * @param msg_size Payload size in bytes.
 */
void akos_thread_post_msg_dynamic(uint8_t des_thread_id, int32_t sig, void *p_content, uint8_t msg_size)
{
    AKOS_CORE_ENTER_CRITICAL();
    if ((task_tcb_list == NULL) || (des_thread_id >= task_tcb_list_len) || (task_tcb_list[des_thread_id] == NULL))
    {
        core_assert(0, "OS_ERR_THREAD_ID_INVALID");
        AKOS_CORE_EXIT_CRITICAL();
        return;
    }
    if (task_tcb_list[des_thread_id] == tcb_curr_ptr)
    {
        // OSUniversalError = OS_ERR_THREAD_POST_MSG_TO_ITSELF;
        core_assert(0, "OS_ERR_THREAD_POST_MSG_TO_ITSELF");
        AKOS_CORE_EXIT_CRITICAL();
        return;
    }

    switch (task_tcb_list[des_thread_id]->state)
    {
    case THREAD_STATE_SUSPENDED_ON_MSG:
        akos_message_queue_put_dynamic(&(task_tcb_list[des_thread_id]->msg_queue),
                                 sig,
                                 p_content,
                                 msg_size);

        /* Is the thread waiting on an event ?  If so remove
         * it from the event list. */
        if (list_item_get_list_contain(&(task_tcb_list[des_thread_id]->event_list_item)) != NULL)
        {
            akos_list_remove(&(task_tcb_list[des_thread_id]->event_list_item));
        }
        add_task_to_rdy_list(task_tcb_list[des_thread_id]);
        if (task_tcb_list[des_thread_id]->prio < tcb_curr_ptr->prio)
        {
            tcb_high_rdy_ptr = task_tcb_list[des_thread_id];

            /*Save state*/
            tcb_high_rdy_ptr->state = THREAD_STATE_RUNNING;

            port_trigger_PendSV();
        }
        AKOS_CORE_EXIT_CRITICAL();
        break;
    case THREAD_STATE_DELAYED_ON_MSG:
        akos_message_queue_put_dynamic(&(task_tcb_list[des_thread_id]->msg_queue),
                                 sig,
                                 p_content,
                                 msg_size);
        /* Is the thread waiting on an event ?  If so remove
         * it from the event list. */
        if (list_item_get_list_contain(&(task_tcb_list[des_thread_id]->state_list_item)) != NULL)
        {
            akos_list_remove(&(task_tcb_list[des_thread_id]->state_list_item));
        }

        add_task_to_rdy_list(task_tcb_list[des_thread_id]);
        if (task_tcb_list[des_thread_id]->prio < tcb_curr_ptr->prio)
        {
            tcb_high_rdy_ptr = task_tcb_list[des_thread_id];

            /*Save state*/
            tcb_high_rdy_ptr->state = THREAD_STATE_RUNNING;

            port_trigger_PendSV();
        }
        AKOS_CORE_EXIT_CRITICAL();
        break;

    default:
        akos_message_queue_put_dynamic(&(task_tcb_list[des_thread_id]->msg_queue),
                                 sig,
                                 p_content,
                                 msg_size);
        AKOS_CORE_EXIT_CRITICAL();
        break;
    }
}

/**
 * @brief Post pure signal message to another thread.
 * @param des_thread_id Destination thread ID.
 * @param sig Message signal.
 */
void akos_thread_post_msg_pure(uint8_t des_thread_id, int32_t sig)
{
    AKOS_CORE_ENTER_CRITICAL();
    if ((task_tcb_list == NULL) || (des_thread_id >= task_tcb_list_len) || (task_tcb_list[des_thread_id] == NULL))
    {
        core_assert(0, "OS_ERR_THREAD_ID_INVALID");
        AKOS_CORE_EXIT_CRITICAL();
        return;
    }
#if 0 /* Under testing */
    if (task_tcb_list[des_thread_id] == tcb_curr_ptr)
    {
        // OSUniversalError = OS_ERR_THREAD_POST_MSG_TO_ITSELF;
        core_assert(0, "OS_ERR_THREAD_POST_MSG_TO_ITSELF");
        AKOS_CORE_EXIT_CRITICAL();
        return;
    }
#endif
    switch (task_tcb_list[des_thread_id]->state)
    {
    case THREAD_STATE_SUSPENDED_ON_MSG:
        akos_message_queue_put_pure(&(task_tcb_list[des_thread_id]->msg_queue), sig);
        /* Is the thread waiting on an event ?  If so remove
         * it from the event list. */
        if (list_item_get_list_contain(&(task_tcb_list[des_thread_id]->event_list_item)) != NULL)
        {
            akos_list_remove(&(task_tcb_list[des_thread_id]->event_list_item));
        }
        add_task_to_rdy_list(task_tcb_list[des_thread_id]);
        if (task_tcb_list[des_thread_id]->prio < tcb_curr_ptr->prio)
        {
            tcb_high_rdy_ptr = task_tcb_list[des_thread_id];

            /*Save state*/
            tcb_high_rdy_ptr->state = THREAD_STATE_RUNNING;

            port_trigger_PendSV();
        }
        AKOS_CORE_EXIT_CRITICAL();
        break;
    case THREAD_STATE_DELAYED_ON_MSG:
        akos_message_queue_put_pure(&(task_tcb_list[des_thread_id]->msg_queue), sig);
        /* Is the thread waiting on an event ?  If so remove
         * it from the event list. */
        if (list_item_get_list_contain(&(task_tcb_list[des_thread_id]->state_list_item)) != NULL)
        {
            akos_list_remove(&(task_tcb_list[des_thread_id]->state_list_item));
        }

        /* Is the thread waiting on an event also?  If so remove
         * it from the event list. */
        if (list_item_get_list_contain(&(task_tcb_list[des_thread_id]->event_list_item)) != NULL)
        {
            akos_list_remove(&(task_tcb_list[des_thread_id]->event_list_item));
        }

        add_task_to_rdy_list(task_tcb_list[des_thread_id]);
        if (task_tcb_list[des_thread_id]->prio < tcb_curr_ptr->prio)
        {
            tcb_high_rdy_ptr = task_tcb_list[des_thread_id];

            /*Save state*/
            tcb_high_rdy_ptr->state = THREAD_STATE_RUNNING;

            port_trigger_PendSV();
        }
        AKOS_CORE_EXIT_CRITICAL();
        break;

    default: /*DELAYED, SUSPEND, RUNNING*/
        akos_message_queue_put_pure(&(task_tcb_list[des_thread_id]->msg_queue), sig);
        AKOS_CORE_EXIT_CRITICAL();
        break;
    }
}

/**
 * @brief Wait for a message on current thread queue.
 * @param time_out Timeout in ticks. Use @ref OS_CFG_DELAY_MAX for infinite wait.
 * @return Message pointer, or NULL on timeout.
 */
msg_t *akos_thread_wait_for_msg(uint32_t time_out)
{
    AKOS_CORE_ENTER_CRITICAL();
    msg_t *p_msg = akos_message_queue_get(&(task_tcb_list[tcb_curr_ptr->id]->msg_queue));
    if (time_out > (uint32_t)0U && p_msg == NULL)
    {
        add_curr_task_to_delay_list(time_out, OS_TRUE); // Can block indefinitely
        if (time_out == OS_CFG_DELAY_MAX)
        {
            tcb_curr_ptr->state = THREAD_STATE_SUSPENDED_ON_MSG;
        }
        else
        {
            tcb_curr_ptr->state = THREAD_STATE_DELAYED_ON_MSG;
        }
        port_trigger_PendSV();
        AKOS_CORE_EXIT_CRITICAL();

        p_msg = akos_message_queue_get(&(task_tcb_list[tcb_curr_ptr->id]->msg_queue));
        return p_msg;
    }
    else
    {
        AKOS_CORE_EXIT_CRITICAL();
        return p_msg;
    }
}
