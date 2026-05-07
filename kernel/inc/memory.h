/*
 * memory.h
 *
 *  Created on: Jun 26, 2024
 *      Author: giahu
 */

#ifndef MEMORY_H
#define MEMORY_H

/**
 * @file memory.h
 * @brief Static-heap allocator APIs.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

    typedef enum
    {
        MEM_STATE_FREE = 0,
        MEM_STATE_BUSY
    } mem_state_t;

    typedef struct mem_blk_header mem_blk_header_t;

	struct mem_blk_header
	{
	    size_t size;
	    mem_state_t state;
	    struct mem_blk_header *next_ptr;
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
