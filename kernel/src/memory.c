/*
*********************************************************************************************************
 *                                            MEMORY
*
 * File    : memory.c
* Version : none
* Author  : JiaHui
*********************************************************************************************************
*/

// clang-format off

/**
 * @file memory.c
 * @brief Fixed-size heap allocator implementation.
 */

#include "config.h"
#include "memory.h"
#include "core.h"

#define ALIGNMENT 						((size_t)4U)
#define MEM_ALIGN(size) 				(size_t)(((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define MIN_SIZE_TO_SPLIT 				((size_t)8U)
#define SIZE_OF_BLOCK_HEADER 			((size_t)MEM_ALIGN(sizeof(mem_blk_header_t)))

static uint8_t mem_heap[OS_CFG_HEAP_SIZE] __attribute__((aligned(ALIGNMENT)));
static mem_blk_header_t mem_blk_start;
static mem_blk_header_t *mem_blk_end_ptr = NULL;
static uint32_t byte_available = 0;

/**
 * @brief Initialize heap block list metadata.
 */
static void memory_heap_init(void)
{
	uint32_t total_heap_size = OS_CFG_HEAP_SIZE;
	uint32_t heap_addr = (uint32_t)mem_heap;

	/* Initialize first block */
	mem_blk_end_ptr = (mem_blk_header_t *)(heap_addr);
	mem_blk_end_ptr->size = total_heap_size - SIZE_OF_BLOCK_HEADER;
	mem_blk_end_ptr->state = MEM_STATE_FREE;
	mem_blk_end_ptr->next_ptr = NULL;

	mem_blk_start.size = 0;
	mem_blk_start.state = MEM_STATE_BUSY;
	mem_blk_start.next_ptr = mem_blk_end_ptr;

	byte_available = total_heap_size - SIZE_OF_BLOCK_HEADER;
}

/**
 * @brief Allocate block from heap.
 * @param size Requested size in bytes.
 * @return Pointer to payload, or NULL if no suitable block.
 */
void *akos_memory_malloc(size_t size)
{
	uint8_t *p_return = NULL;
	mem_blk_header_t *p_block = &mem_blk_start;

	/* Must check the end pointer first */
	if (mem_blk_end_ptr == NULL)
	{
		memory_heap_init();
	}

	/* Then check the size */
	size = MEM_ALIGN(size);
	if ((size == 0) || (size > byte_available))
	{
		core_assert(0, "OS_ERR_MEM_INVALID_SIZE");
		return (void *)p_return;
	}

	p_block = p_block->next_ptr;
	while (p_block != mem_blk_end_ptr)
	{
		if ((p_block->size < size) || (p_block->state == MEM_STATE_BUSY))
		{
			p_block = p_block->next_ptr;
		}
		else
		{
			/* Exit loop */
			break;
		}
	}


	/* Ensure selected block is actually usable (FREE and large enough). */
	if ((p_block->state == MEM_STATE_BUSY) || (p_block->size < size))
	{
		core_assert(0, "OS_ERR_MEM_NO_BLOCK");
		return (void *)p_return;
	}

	if ((p_block->size - size) > (MIN_SIZE_TO_SPLIT + SIZE_OF_BLOCK_HEADER))
	{
		mem_blk_header_t *p_new_block = (mem_blk_header_t *)(((uint8_t *)p_block) + SIZE_OF_BLOCK_HEADER + size);
		p_return = (uint8_t *)p_block + SIZE_OF_BLOCK_HEADER;
		p_new_block->size = p_block->size - size - SIZE_OF_BLOCK_HEADER;
		p_new_block->state = MEM_STATE_FREE;
		p_new_block->next_ptr = p_block->next_ptr;

		if (p_new_block->next_ptr == NULL)
		{
			mem_blk_end_ptr = p_new_block;
		}

		p_block->size = size;
		p_block->state = MEM_STATE_BUSY;
		p_block->next_ptr = p_new_block;
		byte_available -= (size + SIZE_OF_BLOCK_HEADER);
	}
	else
	{
		p_return = ((uint8_t *)p_block + SIZE_OF_BLOCK_HEADER);
		p_block->state = MEM_STATE_BUSY;
		byte_available -= p_block->size;
	}


	return (void *)p_return;
}

/**
 * @brief Free previously allocated heap block.
 * @param p_addr Pointer returned by @ref akos_memory_malloc.
 */
void akos_memory_free(void *p_addr)
{
	if ((mem_blk_end_ptr == NULL))
	{
		core_assert(0, "OS_ERR_MEM_LIST_IS_EMPTY");
		return;
	}

	if (p_addr == NULL)
	{
		core_assert(0, "OS_ERR_MEM_INVALID_ADDRESS");
		return;
	}

	uint8_t is_above_heap_end =
		((uint8_t *)p_addr >= ((uint8_t *)mem_blk_end_ptr + SIZE_OF_BLOCK_HEADER + mem_blk_end_ptr->size));

	uint8_t is_below_heap_start =
		((uint8_t *)p_addr < ((uint8_t *)mem_blk_start.next_ptr + SIZE_OF_BLOCK_HEADER));

	if (is_above_heap_end || is_below_heap_start)
	{
		core_assert(0, "OS_ERR_MEM_INVALID_ADDRESS");
		return;
	}

	mem_blk_header_t *p_block = (mem_blk_header_t *)((uint8_t *)p_addr - SIZE_OF_BLOCK_HEADER);
	mem_blk_header_t *p_block_temp = &mem_blk_start;
	mem_blk_header_t *p_prev_block = p_block_temp;

	while ((p_block_temp != mem_blk_end_ptr) && (p_block_temp != p_block))
	{
		p_prev_block = p_block_temp;
		p_block_temp = p_block_temp->next_ptr;
	}

	if (p_block_temp != p_block)
	{
		core_assert(0, "OS_ERR_MEM_INVALID_ADDRESS");
		return;
	}

	if (p_block_temp->state == MEM_STATE_FREE)
	{
		core_assert(0, "OS_ERR_MEM_DOUBLE_FREE");
		return;
	}

	byte_available += p_block_temp->size;
	p_block_temp->state = MEM_STATE_FREE;

	/* Merge next block */
	if ((p_block_temp->next_ptr != NULL) && (p_block_temp->next_ptr->state == MEM_STATE_FREE))
	{
		byte_available += SIZE_OF_BLOCK_HEADER;
		p_block_temp->size += p_block_temp->next_ptr->size + SIZE_OF_BLOCK_HEADER;
		p_block_temp->next_ptr = p_block_temp->next_ptr->next_ptr;
		if (p_block_temp->next_ptr == NULL)
		{
			mem_blk_end_ptr = p_block_temp;
		}
	}

	/* Merge previous block */
	if ((p_prev_block->state == MEM_STATE_FREE) && (p_prev_block != &mem_blk_start))
	{
		if (mem_blk_end_ptr == p_block_temp)
		{
			mem_blk_end_ptr = p_prev_block;
		}

		byte_available += SIZE_OF_BLOCK_HEADER;
		p_prev_block->size += p_block_temp->size + SIZE_OF_BLOCK_HEADER;
		p_prev_block->next_ptr = p_block_temp->next_ptr;
	}
}
