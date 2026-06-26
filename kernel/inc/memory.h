/**
  ******************************************************************************
  * @file    memory.h
  * @brief   Static-heap allocator APIs.
  *
  * @author  Snoopy3921 - AK Foundation
  * @date    Created: 2026-06-11
  * @date    Updated: 2026-06-26
  *
  * @module  AKOS
  ******************************************************************************
  */

#ifndef __MEMORY_H__
#define __MEMORY_H__

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Enums ---------------------------------------------------------------------*/
/**
 * @enum mem_state_t
 * @brief Runtime state of a heap block.
 */
typedef enum
{
    MEM_STATE_FREE = 0, /**< Block is available for allocation. */
    MEM_STATE_BUSY      /**< Block is currently allocated. */
} mem_state_t;

/* Typedefs ------------------------------------------------------------------*/
/** @brief Heap block header type. */
typedef struct mem_blk_header mem_blk_header_t;

/* Structs -------------------------------------------------------------------*/
/**
 * @struct mem_blk_header
 * @brief Metadata stored before each heap block.
 */
struct mem_blk_header
{
    size_t size;                         /**< Payload size in bytes. */
    mem_state_t state;                   /**< Allocation state of this block. */
    struct mem_blk_header *next_ptr;     /**< Next block in heap block list. */
};

/* Function prototypes -------------------------------------------------------*/
/**
 * @brief Allocate memory from OS heap.
 * @param size Requested size in bytes.
 * @return Pointer to allocated block, or NULL on failure.
 */
void *akos_memory_malloc(size_t size);

/**
 * @brief Free previously allocated memory block.
 * @param p_addr Pointer returned by @ref akos_memory_malloc.
 */
void akos_memory_free(void *p_addr);

#ifdef __cplusplus
}
#endif

#endif /* __MEMORY_H__ */
