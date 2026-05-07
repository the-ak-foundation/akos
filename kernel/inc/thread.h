/*
 * thread.h
 *
 *  Created on: Jun 26, 2024
 *      Author: giahu
 */

#ifndef THREAD_H
#define THREAD_H

/**
 * @file thread.h
 * @brief Thread scheduling and thread messaging APIs.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include "config.h"

#include "message.h"

#define AKOS_THREAD_DEFINE(_name, _id, _entry, _arg, _prio, _queue_size, _stack_size) \
  const thread_t _name __attribute__((used, section("task_desc"))) = {                \
      .id = (thread_id_t)(_id),                                                       \
      .pf_thread = (thread_func_t)(_entry),                                           \
      .p_arg = (void *)(_arg),                                                        \
      .prio = (uint8_t)(_prio),                                                       \
      .queue_size = (size_t)(_queue_size),                                            \
      .stack_size = (size_t)(_stack_size),                                            \
  }

  /**
   * @enum thread_state_t
   * @brief Runtime state of a thread control block.
   */
  typedef enum
  {
    THREAD_STATE_RUNNING = 0,     /**< Currently executing thread. */
    THREAD_STATE_READY,           /**< Ready to run, waiting for CPU. */
    THREAD_STATE_DELAYED,          /**< Blocked until tick timeout expires. */
    THREAD_STATE_SUSPENDED,        /**< Suspended explicitly by kernel/app logic. */
    THREAD_STATE_SUSPENDED_ON_MSG, /**< Waiting for message without timeout. */
    THREAD_STATE_DELAYED_ON_MSG    /**< Waiting for message with timeout. */
  } thread_state_t;

  typedef struct thread_tcb *thread_handle_t; /**< Opaque pointer to internal thread control block. */
  typedef struct thread thread_t;           /**< Static thread descriptor used by @ref AKOS_THREAD_DEFINE. */

  /**
   * @typedef thread_func_t
   * @brief Thread entry function signature.
   * @param p_arg User-provided thread argument.
   */
  typedef void (*thread_func_t)(void *p_arg);

  /**
   * @typedef thread_id_t
   * @brief Numeric thread identifier type.
   */
  typedef uint8_t thread_id_t;

  /**
   * @struct thread
   * @brief Static thread descriptor used during thread table registration.
   */
  struct thread
  {
    thread_id_t id;         /**< Application-level thread ID; keep these dense from 0..N-1. */
    thread_func_t pf_thread; /**< Thread entry function. */
    void *p_arg;            /**< Argument passed to thread entry. */
    uint8_t prio;           /**< Scheduler priority (lower value means higher priority). */
    size_t queue_size;       /**< Message queue capacity for this thread. */
    size_t stack_size;       /**< Requested stack size in 32-bit words. */
  };

  /**
   * @brief Get current system tick.
   * @return Tick counter.
   */
  uint32_t akos_thread_get_tick(void);

  /**
   * @brief Create all statically defined threads and kernel internal threads.
   */
  void akos_thread_register_static_threads(void);

  /**
   * @brief Get number of application threads defined via @ref AKOS_THREAD_DEFINE.
   * @return Number of app threads linked into the image. App IDs are 0..count-1.
   */
  uint8_t akos_thread_get_app_thread_count(void);

  /**
   * @brief Get the runtime thread ID assigned to the idle thread.
   * @return Idle thread ID.
   */
  uint8_t akos_thread_get_idle_thread_id(void);

  /**
   * @brief Get the runtime thread ID assigned to the timer thread.
   * @return Timer thread ID.
   */
  uint8_t akos_thread_get_timer_thread_id(void);

  /**
   * @brief Tick handler routine called from SysTick.
   * @return OS_TRUE if context switch is needed, otherwise OS_FALSE.
   */
  uint8_t akos_thread_increment_tick(void);

  /**
   * @brief Delay current thread by specified ticks.
   * @param tick_to_delay Delay duration in ticks.
   */
  void akos_thread_delay(const uint32_t tick_to_delay);

  /**
   * @brief Start scheduler state.
   */
  void akos_thread_start(void);

  /**
   * @brief Post dynamic message to destination thread.
   * @param des_thread_id Destination thread ID.
   * @param sig Message signal.
   * @param p_content Payload pointer.
   * @param msg_size Payload size in bytes.
   */
  void akos_thread_post_msg_dynamic(uint8_t des_thread_id, int32_t sig, void *p_content, uint8_t msg_size);

  /**
   * @brief Post pure signal message to destination thread.
   * @param des_thread_id Destination thread ID.
   * @param sig Message signal.
   */
  void akos_thread_post_msg_pure(uint8_t des_thread_id, int32_t sig);

  /**
   * @brief Wait for message from current thread queue.
   * @param time_out Timeout in ticks. Use @ref OS_CFG_DELAY_MAX for infinite wait.
   * @return Message pointer, or NULL on timeout.
   */
  msg_t *akos_thread_wait_for_msg(uint32_t time_out);

#ifdef __cplusplus
}
#endif
#endif /* THREAD_H */
