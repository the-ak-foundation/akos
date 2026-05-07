/**
 * @file timer.c
 * @brief Software timer pool, lists, and processing loop.
 */

#include "core.h"
#include "timer.h"
#include "thread.h"
#include "message.h"
#include "list.h"

static ak_timer_t timer_pool[OS_CFG_TIMER_POOL_SIZE];
static ak_timer_t *free_list_timer_pool;
static uint8_t timer_pool_used;

static uint32_t timer_counter;

static list_t timer_list_1;
static list_t timer_list_2;

static list_t *volatile timer_list_ptr; /* Current */
static list_t *volatile overflow_timer_list_ptr;

static volatile uint32_t next_tick_to_unblock_timer = (uint32_t)OS_CFG_DELAY_MAX;

/**
 * @brief Initialize timer object free-list.
 */
static void timer_pool_init()
{
    uint8_t index;

    free_list_timer_pool = (ak_timer_t *)timer_pool;

    for (index = 0; index < OS_CFG_TIMER_POOL_SIZE; index++)
    {
        if (index == (OS_CFG_TIMER_POOL_SIZE - 1))
        {
            timer_pool[index].next = NULL;
        }
        else
        {
            timer_pool[index].next = (ak_timer_t *)&timer_pool[index + 1];
        }
    }

    timer_pool_used = 0;
}

/**
 * @brief Initialize timer active/overflow lists.
 */
static void init_timer_lists(void)
{
    /* Initialize lists */
    akos_list_init(&timer_list_1);
    akos_list_init(&timer_list_2);
    timer_list_ptr = &timer_list_1;
    overflow_timer_list_ptr = &timer_list_2;
    /********************/
}

/**
 * @brief Swap active and overflow timer lists on tick wrap.
 */
static void timer_switch_lists()
{
    list_t *p_list_temp;
    p_list_temp = timer_list_ptr;
    timer_list_ptr = overflow_timer_list_ptr;
    overflow_timer_list_ptr = p_list_temp;

    if (list_is_empty(timer_list_ptr) == OS_TRUE)
    {
        next_tick_to_unblock_timer = OS_CFG_DELAY_MAX;
    }
    else
    {
        next_tick_to_unblock_timer = 0u;
    }
}
/**
 * @brief Insert timer into active or overflow list by trigger tick.
 * @param p_timer Timer object.
 */
static void add_timer_to_list(ak_timer_t *p_timer)
{
    AKOS_CORE_ENTER_CRITICAL();
    {
        const uint32_t const_tick = akos_thread_get_tick();
        uint32_t tick_to_trigger = list_item_get_value(&(p_timer->timer_list_item));
        if (tick_to_trigger < const_tick)
        {
            /* Wake time has overflowed.  Place this item in the overflow
             * list. */
            akos_list_insert(overflow_timer_list_ptr, &(p_timer->timer_list_item));
        }
        else
        {
            /* The wake time has not overflowed, so the current block list
             * is used. */
            akos_list_insert(timer_list_ptr, &(p_timer->timer_list_item));
        }
    }
    AKOS_CORE_EXIT_CRITICAL();
}
/**
 * @brief Recompute next timer-unblock tick.
 */
static void update_next_tick_to_unblock()
{
    ak_timer_t *p_timer;
    uint32_t item_value;
    if (list_is_empty(timer_list_ptr) == OS_TRUE)
    {
        next_tick_to_unblock_timer = OS_CFG_DELAY_MAX;
    }
    else
    {
        p_timer = list_get_owner_of_head_item(timer_list_ptr);
        item_value = list_item_get_value(&(p_timer->timer_list_item));
        if (item_value < next_tick_to_unblock_timer)
        {
            next_tick_to_unblock_timer = item_value;
        }
    }
}

/**
 * @brief Create a timer object from timer pool.
 * @param id Timer ID.
 * @param sig Signal posted on timeout.
 * @param func_cb Optional callback run on timeout.
 * @param des_thread_id Destination thread ID for signal posting.
 * @param period Period in ticks for periodic timer.
 * @param type Timer type.
 * @return Timer pointer, or NULL on failure.
 */
ak_timer_t *akos_timer_create(timer_id_t id, int32_t sig, timer_cb func_cb, uint8_t des_thread_id, uint32_t period, timer_type_t type)
{
    /*It is important to set value for timer_list_item, because it holds time stamp*/
    if (des_thread_id >= akos_thread_get_app_thread_count())
    {
        // OSUniversalError = OS_ERR_DES_TASK_ID_INVALID;
        core_assert(0, "OS_ERR_DES_THREAD_ID_INVALID");
        return NULL;
    }
    if (des_thread_id == akos_thread_get_timer_thread_id())
    {
        // OSUniversalError = OS_ERR_CAN_NOT_SET_DES_TO_ITSELF;
        core_assert(0, "OS_ERR_CAN_NOT_SET_DES_TO_ITSELF");
        return NULL;
    }
    if (period == 0u && type == TIMER_PERIODIC)
    {
        // OSUniversalError = OS_ERR_TIMER_NOT_ACECPT_ZERO_PERIOD;
        core_assert(0, "OS_ERR_TIMER_NOT_ACECPT_ZERO_PERIOD");
        return NULL;
    }
    ak_timer_t *p_timer;
    AKOS_CORE_ENTER_CRITICAL();
    if ((timer_pool_used >= OS_CFG_TIMER_POOL_SIZE) || (free_list_timer_pool == NULL))
    {
        AKOS_CORE_EXIT_CRITICAL();
        core_assert(0, "OS_ERR_TIMER_POOL_IS_FULL");
        return NULL;
    }
    p_timer = free_list_timer_pool;
    free_list_timer_pool = p_timer->next;
    timer_pool_used++;
    AKOS_CORE_EXIT_CRITICAL();

    p_timer->id = id;
    p_timer->sig = sig;
    p_timer->func_cb = func_cb;
    p_timer->des_thread_id = des_thread_id;

    switch (type)
    {
    case TIMER_ONE_SHOT:
        /* code */
        p_timer->period = 0;
        break;
    case TIMER_PERIODIC:
        p_timer->period = period;
        break;
    default:
        break;
    }
    /* Make connection */
    akos_list_item_init(&(p_timer->timer_list_item));
    list_item_set_owner(&(p_timer->timer_list_item), (void *)p_timer);

    // list_item_set_value(&(p_timer->timer_list_item), period + akos_thread_get_tick());
    // add_timer_to_list(p_timer);
    // update_next_tick_to_unblock();
    // akos_thread_post_msg_pure(OS_CFG_TIMER_TASK_ID, TIMER_CMD_UPDATE);
    return p_timer;
}
/**
 * @brief Remove timer from lists and return it to pool.
 * @param p_timer Timer object.
 */
void akos_timer_remove(ak_timer_t *p_timer)
{
    AKOS_CORE_ENTER_CRITICAL();
    if (timer_pool_used == 0u)
    {
        AKOS_CORE_EXIT_CRITICAL();
        core_assert(0, "OS_ERR_TIMER_POOL_UNDERFLOW");
        return;
    }

    p_timer->next = free_list_timer_pool;
    free_list_timer_pool = p_timer;
    timer_pool_used--;

    if (list_item_get_list_contain(&(p_timer->timer_list_item)) != NULL)
        akos_list_remove(&(p_timer->timer_list_item));

    AKOS_CORE_EXIT_CRITICAL();
}

/**
 * @brief Initialize timer module state.
 */
void akos_timer_init(void)
{
    timer_pool_init();
    init_timer_lists();
    next_tick_to_unblock_timer = OS_CFG_DELAY_MAX;
    /*TODO: Create a thread for timer here, or maybe in thread.c and call akos_timer_processing*/
}
/**
 * @brief Timer-task processing function.
 *
 * Processes expired timers, executes callbacks/posts signals, then blocks
 * until the next timer event (or a wakeup message).
 */
void akos_timer_processing()
{
    msg_t *p_msg;
    ak_timer_t *p_timer;
    uint32_t item_value;
    static uint32_t last_time = (uint32_t)0U;
    uint32_t time_now = akos_thread_get_tick();
    if (time_now < last_time)
    {
        /* Overflown */
        timer_switch_lists();
    }
    if (time_now >= next_tick_to_unblock_timer)
    {
        for (;;)
        {
            if (list_is_empty(timer_list_ptr) == OS_TRUE)
            {
                next_tick_to_unblock_timer = OS_CFG_DELAY_MAX;
                break;
            }
            else
            {
                p_timer = list_get_owner_of_head_item(timer_list_ptr);
                item_value = list_item_get_value(&(p_timer->timer_list_item));
                if (item_value > time_now)
                {
                    /* Stop condition */
                    next_tick_to_unblock_timer = item_value;
                    break;
                }
                akos_list_remove(&(p_timer->timer_list_item));

                if (p_timer->func_cb != NULL)
                    p_timer->func_cb();
                else
                    akos_thread_post_msg_pure(p_timer->des_thread_id, p_timer->sig);
                if (p_timer->period != 0)
                {
                    list_item_set_value(&(p_timer->timer_list_item), p_timer->period + time_now);
                    add_timer_to_list(p_timer);
                }
                else
                    akos_timer_remove(p_timer); /* One shot */
                update_next_tick_to_unblock();
            }
        }
    }
    last_time = time_now;
    p_msg = akos_thread_wait_for_msg(next_tick_to_unblock_timer - time_now);
    if (p_msg != NULL)
        akos_message_free(p_msg);
}

/**
 * @brief Start timer with initial delay.
 * @param p_timer Timer object.
 * @param tick_to_wait Initial delay in ticks.
 */
void akos_timer_start(ak_timer_t *p_timer, uint32_t tick_to_wait)
{
    uint32_t time_now = akos_thread_get_tick();

    list_item_set_value(&(p_timer->timer_list_item), tick_to_wait + time_now);

    add_timer_to_list(p_timer);
    update_next_tick_to_unblock();
    akos_thread_post_msg_pure(akos_thread_get_timer_thread_id(), 0); // Dummy signal
}

/**
 * @brief Reset a running timer to its next period.
 * @param p_timer Timer object.
 */
void akos_timer_reset(ak_timer_t *p_timer)
{
    if (list_item_get_list_contain(&(p_timer->timer_list_item)) == NULL)
    {
        // OSUniversalError = OS_ERR_TIMER_IS_NOT_RUNNING;
        core_assert(0, "OS_ERR_TIMER_IS_NOT_RUNNING");
        return;
    }
    uint32_t time_now = akos_thread_get_tick();
    akos_list_remove(&(p_timer->timer_list_item));
    if (p_timer->period != 0)
    {
        list_item_set_value(&(p_timer->timer_list_item), p_timer->period + time_now);
        add_timer_to_list(p_timer);
    }
    else
        akos_timer_remove(p_timer); /* One shot */
    update_next_tick_to_unblock();
    akos_thread_post_msg_pure(akos_thread_get_timer_thread_id(), 0); // Dummy signal
}
