/**
 * @file message.c
 * @brief Message pool and queue implementation.
 */

#include "message.h"
#include "memory.h"
#include "core.h"

#include <stdint.h>
#include <string.h>

static msg_t msg_pool[OS_CFG_MSG_POOL_SIZE];
static msg_t *free_list_msg_pool;
static uint8_t msg_pool_used;

/**
 * @brief Initialize free-list of preallocated message objects.
 */
static void msg_pool_init(void)
{
    AKOS_CORE_ENTER_CRITICAL();
    uint8_t index;

    free_list_msg_pool = (msg_t *)msg_pool;

    for (index = 0; index < OS_CFG_MSG_POOL_SIZE; index++)
    {
        if (index == (OS_CFG_MSG_POOL_SIZE - 1))
        {
            msg_pool[index].next = NULL;
        }
        else
        {
            msg_pool[index].next = (msg_t *)&msg_pool[index + 1];
        }
    }

    msg_pool_used = 0;

    AKOS_CORE_EXIT_CRITICAL();
}
/**
 * @brief Initialize message subsystem.
 */
void akos_message_init(void)
{
    msg_pool_init();
}

/**
 * @brief Free a message and return resources to pools.
 * @param p_msg Message object to release.
 */
void akos_message_free(msg_t *p_msg)
{
    AKOS_CORE_ENTER_CRITICAL();

    p_msg->next = free_list_msg_pool;
    free_list_msg_pool = p_msg;
    if (p_msg->type == MSG_TYPE_DYNAMIC)
    {
        akos_memory_free(p_msg->content_ptr);
    }
    msg_pool_used--;

    AKOS_CORE_EXIT_CRITICAL();
}

/**
 * @brief Initialize a message queue.
 * @param p_msg_q Queue object.
 * @param size Maximum queue length.
 */
void akos_message_queue_init(msg_queue_t *p_msg_q,
                       uint8_t size)
{
    p_msg_q->head_ptr = NULL;
    p_msg_q->tail_ptr = NULL;
    p_msg_q->size_max = size;
    p_msg_q->size_curr = 0u;
}

/**
 * @brief Enqueue dynamic message and copy payload.
 * @param p_msg_q Destination queue.
 * @param sig Signal value.
 * @param p_content Payload source pointer.
 * @param size Payload size in bytes.
 */
void akos_message_queue_put_dynamic(msg_queue_t *p_msg_q,
                              int32_t sig,
                              void *p_content,
                              uint8_t size)
{
    AKOS_CORE_ENTER_CRITICAL();
    msg_t *p_msg;
    msg_t *p_msg_tail;
    if (p_msg_q->size_curr >= p_msg_q->size_max)
    {
        // OSUniversalError = OS_ERR_MSG_QUEUE_IS_FULL;
        core_assert(0, "OS_ERR_MSG_QUEUE_IS_FULL");
        AKOS_CORE_EXIT_CRITICAL();
        return;
    }
    if (msg_pool_used >= OS_CFG_MSG_POOL_SIZE)
    {
        // OSUniversalError = OS_ERR_MSG_POOL_IS_FULL;
        core_assert(0, "OS_ERR_MSG_POOL_IS_FULL");
        AKOS_CORE_EXIT_CRITICAL();
        return;
    }

    p_msg = free_list_msg_pool;
    free_list_msg_pool = p_msg->next;
    msg_pool_used++;

    p_msg->type = MSG_TYPE_DYNAMIC;
    p_msg->sig = sig;
    p_msg->size = size;
    p_msg->content_ptr = (uint8_t *)akos_memory_malloc(size);
    if (p_msg->content_ptr == NULL)
    {
        p_msg->next = free_list_msg_pool;
        free_list_msg_pool = p_msg;
        msg_pool_used--;
        core_assert(0, "OS_ERR_MEM_NO_BLOCK");
        AKOS_CORE_EXIT_CRITICAL();
        return;
    }
    memcpy(p_msg->content_ptr, p_content, size);

    if (p_msg_q->size_curr == 0u) /* Is this first message placed in the queue? */
    {
        p_msg_q->head_ptr = p_msg; /* Yes */
        p_msg_q->tail_ptr = p_msg;
        p_msg_q->size_curr = 1u;
        p_msg->next = NULL;
    }
    else
    {
        p_msg_tail = p_msg_q->tail_ptr;
        p_msg_tail->next = p_msg;
        p_msg_q->tail_ptr = p_msg;
        p_msg->next = NULL;

        p_msg_q->size_curr++;
    }

    AKOS_CORE_EXIT_CRITICAL();
}

/**
 * @brief Enqueue pure message (signal only).
 * @param p_msg_q Destination queue.
 * @param sig Signal value.
 */
void akos_message_queue_put_pure(msg_queue_t *p_msg_q, int32_t sig)
{
    AKOS_CORE_ENTER_CRITICAL();
    msg_t *p_msg;
    msg_t *p_msg_tail;
    if (p_msg_q->size_curr >= p_msg_q->size_max)
    {
        // OSUniversalError = OS_ERR_MSG_QUEUE_IS_FULL;
        core_assert(0, "OS_ERR_MSG_QUEUE_IS_FULL");
        AKOS_CORE_EXIT_CRITICAL();
        return;
    }
    if (msg_pool_used >= OS_CFG_MSG_POOL_SIZE)
    {
        /* This states that u forget to free msg somewhere.*/
        // OSUniversalError = OS_ERR_MSG_POOL_IS_FULL;
        core_assert(0, "OS_ERR_MSG_POOL_IS_FULL");
        AKOS_CORE_EXIT_CRITICAL();
        return;
    }

    p_msg = free_list_msg_pool;
    free_list_msg_pool = p_msg->next;
    msg_pool_used++;

    if (p_msg_q->size_curr == 0u) /* Is this first message placed in the queue? */
    {
        p_msg_q->head_ptr = p_msg; /* Yes */
        p_msg_q->tail_ptr = p_msg;
        p_msg_q->size_curr = 1u;
        p_msg->next = NULL;
    }
    else
    {
        p_msg_tail = p_msg_q->tail_ptr;
        p_msg_tail->next = p_msg;
        p_msg_q->tail_ptr = p_msg;
        p_msg->next = NULL;

        p_msg_q->size_curr++;
    }

    p_msg->type = MSG_TYPE_PURE;
    p_msg->sig = sig;

    AKOS_CORE_EXIT_CRITICAL();
}

/**
 * @brief Dequeue next message from queue.
 * @param p_msg_q Source queue.
 * @return Message pointer, or NULL when empty.
 */
msg_t *akos_message_queue_get(msg_queue_t *p_msg_q)
{
    msg_t *p_msg;

    if (p_msg_q->size_curr == 0u)
    {
        // OSUniversalError = OS_ERR_MSG_QUEUE_IS_EMPTY;
        // core_assert(0);
        return NULL;
    }

    p_msg = p_msg_q->head_ptr;

    p_msg_q->head_ptr = p_msg->next;

    if (p_msg_q->size_curr == 1u) /* Are there any more messages in the queue? */
    {
        p_msg_q->head_ptr = NULL;
        p_msg_q->tail_ptr = NULL;
        p_msg_q->size_curr = 0u;
    }
    else
    {
        p_msg_q->size_curr--; /* Yes, One less message in the queue */
    }

    return (p_msg);
}

/**
 * @brief Access payload pointer and payload size from dynamic message.
 * @param p_msg Message object.
 * @param p_msg_size Output payload size in bytes.
 * @return Payload pointer.
 */
void *akos_message_get_dynamic_data(msg_t *p_msg,
                              uint8_t *p_msg_size)
{
    *p_msg_size = p_msg->size;
    return p_msg->content_ptr;
}

/**
 * @brief Dequeue next pure message from queue.
 * @param p_msg_q Source queue.
 * @return Message pointer, or NULL when empty.
 */
msg_t *akos_message_queue_get_pure(msg_queue_t *p_msg_q)
{
    msg_t *p_msg;

    if (p_msg_q->size_curr == 0u)
    {
        // OSUniversalError = OS_ERR_MSG_QUEUE_IS_EMPTY;
        // core_assert(0);
        return NULL;
    }

    p_msg = p_msg_q->head_ptr;

    p_msg_q->head_ptr = p_msg->next;

    if (p_msg_q->size_curr == 1u) /* Are there any more messages in the queue? */
    {
        p_msg_q->head_ptr = NULL;
        p_msg_q->tail_ptr = NULL;
        p_msg_q->size_curr = 0u;
    }
    else
    {
        p_msg_q->size_curr--; /* Yes, One less message in the queue */
    }

    return (p_msg);
}

/**
 * @brief Read signal value from pure message.
 * @param p_msg Message object.
 * @return Signal value.
 */
int32_t akos_message_get_pure_data(msg_t *p_msg)
{
    return p_msg->sig;
}
