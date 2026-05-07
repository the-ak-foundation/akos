/*
 * message.h
 *
 *  Created on: Jun 26, 2024
 *      Author: giahu
 */

#ifndef MESSAGE_H
#define MESSAGE_H

/**
 * @file message.h
 * @brief Message object and message queue APIs.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include "config.h"

#include <stdint.h>
#include <stdbool.h>

    /** @brief Message node type. */
    typedef struct msg msg_t;
    /** @brief Message queue type. */
    typedef struct msg_queue msg_queue_t;
    /** @brief Internal message-pool type. */
    typedef struct msg_pool msg_pool_t;

    /**
     * @enum msg_type_t
     * @brief Message payload model.
     */
    typedef enum
    {
        MSG_TYPE_PURE = 0, /**< Signal-only message, no payload buffer. */
        MSG_TYPE_DYNAMIC   /**< Message with copied payload buffer. */
    } msg_type_t;

    /**
     * @struct msg
     * @brief Message object stored in queues.
     */
    struct msg
    {
        msg_t *next;          /**< Next message in queue or free list. */
        uint8_t size;         /**< Payload size in bytes. */
        int32_t sig;          /**< Signal identifier/value. */
        uint8_t *content_ptr; /**< Dynamic payload buffer pointer, if any. */
        msg_type_t type;      /**< Payload model of this message. */
        uint8_t src_thread_id; /**< Source thread ID. */
        uint8_t des_thread_id; /**< Destination thread ID. */
    };

    /**
     * @struct msg_queue
     * @brief FIFO queue metadata for thread messaging.
     */
    struct msg_queue
    {
        msg_t *head_ptr;    /**< Queue head. */
        msg_t *tail_ptr;    /**< Queue tail. */
        uint8_t size_max;   /**< Maximum message count allowed. */
        uint8_t size_curr;  /**< Current message count. */
    };

	    /**
	     * @brief Initialize global message pool.
	     */
	    void akos_message_init(void);

	    /**
	     * @brief Return message to message pool.
	     * @param p_msg Message to free.
	     */
	    void akos_message_free(msg_t *p_msg);

	    /**
	     * @brief Initialize message queue.
	     * @param p_msg_q Queue object.
	     * @param size Maximum queue depth.
	     */
	    void akos_message_queue_init(msg_queue_t *p_msg_q, uint8_t size);

	    /**
	     * @brief Enqueue dynamic message with payload copy.
	     * @param p_msg_q Destination queue.
	     * @param sig Message signal.
	     * @param p_content Payload pointer.
	     * @param size Payload size in bytes.
	     */
	    void akos_message_queue_put_dynamic(msg_queue_t *p_msg_q, int32_t sig, void *p_content, uint8_t size);

	    /**
	     * @brief Enqueue pure signal message.
	     * @param p_msg_q Destination queue.
	     * @param sig Message signal.
	     */
	    void akos_message_queue_put_pure(msg_queue_t *p_msg_q, int32_t sig);

	    /**
	     * @brief Dequeue next message.
	     * @param p_msg_q Source queue.
	     * @return Message pointer, or NULL if queue is empty.
	     */
	    msg_t *akos_message_queue_get(msg_queue_t *p_msg_q);

	    /**
	     * @brief Get payload data from dynamic message.
	     * @param p_msg Message pointer.
	     * @param p_msg_size Output payload size.
	     * @return Payload pointer.
	     */
	    void *akos_message_get_dynamic_data(msg_t *p_msg, uint8_t *p_msg_size);

	    /**
	     * @brief Dequeue next pure message.
	     * @param p_msg_q Source queue.
	     * @return Message pointer, or NULL if queue is empty.
	     */
	    msg_t *akos_message_queue_get_pure(msg_queue_t *p_msg_q);

	    /**
	     * @brief Get signal field from pure message.
	     * @param p_msg Message pointer.
	     * @return Signal value.
	     */
	    int32_t akos_message_get_pure_data(msg_t *p_msg);

#ifdef __cplusplus
}
#endif
#endif /* MESSAGE_H */
