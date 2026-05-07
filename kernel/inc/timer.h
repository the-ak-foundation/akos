/*
 * timer.h
 *
 *  Created on: Dec 6, 2024
 *      Author: giahu
 */

#ifndef TIMER_H
#define TIMER_H

/**
 * @file timer.h
 * @brief Software timer APIs.
 */

#ifdef __cplusplus
extern "C"
{
#endif
#include "list.h"
#include <stdint.h>
#include <stdbool.h>

    /** @brief Software timer object type. */
    typedef struct ak_timer ak_timer_t;
    /** @brief Internal timer pool type. */
    typedef struct timer_pool timer_pool_t;
    /** @brief Timer identifier type. */
    typedef uint8_t timer_id_t;

    /**
     * @typedef timer_cb
     * @brief Timer callback signature.
     *
     * Callback runs in timer-thread context; keep execution short and non-blocking.
     */
    typedef void (*timer_cb)();

    /**
     * @enum timer_type_t
     * @brief Timer mode.
     */
    typedef enum
    {
        TIMER_ONE_SHOT, /**< Expires once and then stops. */
        TIMER_PERIODIC  /**< Reloads using @ref timer.period. */
    } timer_type_t;

    /**
     * @struct timer
     * @brief Timer object managed by the software timer subsystem.
     */
    struct ak_timer
    {
        ak_timer_t *next;          /**< Next timer in free-list. */
        timer_id_t id;             /**< User-defined timer identifier. */
        list_item_t timer_list_item; /**< List item used in active/overflow timer lists. */
        int32_t sig;               /**< Signal posted to destination thread on expiry. */
        uint8_t des_thread_id;     /**< Destination thread ID for signal posting. */
        timer_cb func_cb;          /**< Optional callback executed on timer expiry. */
        uint32_t period;           /**< Period in ticks (0 for one-shot timers). */
    };

	    /**
	     * @brief Initialize timer module.
	     */
	    void akos_timer_init(void); /* Runs on kernel init */

	    /**
     * @brief Timer thread processing loop.
	     */
	    void akos_timer_processing(); /* Runs on timer thread */

    /* These APIs are called from application threads. */
	    /**
	     * @brief Create timer object from timer pool.
	     * @param id Timer ID.
	     * @param sig Signal posted when timer expires.
	     * @param func_cb Optional callback executed on expiry.
     * @param des_thread_id Destination thread ID for signal posting.
	     * @param period Period in ticks for periodic timers.
	     * @param type Timer type (one-shot or periodic).
	     * @return Created timer pointer, or NULL on failure.
	     */
	    ak_timer_t *akos_timer_create(timer_id_t id, int32_t sig, timer_cb func_cb, uint8_t des_thread_id, uint32_t period, timer_type_t type);

	    /**
	     * @brief Start a timer.
	     * @param p_timer Timer pointer.
	     * @param tick_to_wait Initial delay in ticks.
	     */
	    void akos_timer_start(ak_timer_t *p_timer, uint32_t tick_to_wait);
	    /**
	     * @brief Reset a running timer.
	     * @param p_timer Timer pointer.
	     */
	    void akos_timer_reset(ak_timer_t *p_timer);
	    /**
	     * @brief Remove timer from active lists and return to pool.
	     * @param p_timer Timer pointer.
	     */
	    void akos_timer_remove(ak_timer_t *p_timer);

#ifdef __cplusplus
}
#endif
#endif /* TIMER_H */
