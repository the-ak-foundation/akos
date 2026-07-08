/*!
 * @file	memory.h
 * @author	Snoopy3921 - AK Foundation
 * @date	2026/05/08
 * @module	AKOS
 * @brief	Static-heap allocator APIs.
 */

#ifndef MEMORY_H
#define MEMORY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Heap block allocation state.
 */
typedef enum
{
	MEM_STATE_FREE = 0,	                /**< Heap block is available for allocation. */
	MEM_STATE_BUSY		                /**< Heap block is currently allocated. */
} mem_state_t;

/** @brief Heap block header type. */
typedef struct mem_blk_header mem_blk_header_t;

/**
 * @struct mem_blk_header
 * @brief Metadata stored before each heap block.
 */
struct mem_blk_header
{
	size_t size;						/**< Payload size in bytes for this block. */
	mem_state_t state;					/**< Allocation state of this block. */
	struct mem_blk_header *next_ptr;	/**< Next block in the heap block list. */
};

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
#endif /* MEMORY_H */
