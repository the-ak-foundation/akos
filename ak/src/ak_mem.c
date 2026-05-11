#include "ak_mem.h"
#include "ak_cfg.h"
#include "ak_list.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define AK_BLOCK_HEADER_SIZE AK_ALIGN_CEIL(sizeof(_ak_mem_block_header_t))

typedef enum : uint8_t { AK_BLOCK_FREE, AK_BLOCK_BUSY } _ak_mem_block_state_t;

typedef struct {
    ak_list_node_t node;
    size_t size; /* size including header */
    _ak_mem_block_state_t state;
} _ak_mem_block_header_t;

static size_t _ak_mem_avail_size;
static _ak_mem_block_header_t* _ak_mem_first_block_p;
static _ak_mem_block_header_t* _ak_mem_last_block_p;
static void* _ak_mem_start_p;

/* These two must stick together. */
static _ak_mem_block_header_t
    __attribute__((aligned(AK_CFG_HEAP_ALIGN))) _ak_mem_initial_block = {
        .node = {.prev = NULL, .next = NULL},
        .size = AK_BLOCK_HEADER_SIZE + AK_CFG_HEAP_SIZE,
        .state = AK_BLOCK_FREE};
static uint8_t
    __attribute__((aligned(AK_CFG_HEAP_ALIGN))) _ak_mem_arr[AK_CFG_HEAP_SIZE];

static inline _ak_mem_block_header_t* _ak_mem_find_fit(size_t size);
static inline _ak_mem_block_header_t* _ak_mem_get_block(void* addr);
static inline _ak_mem_block_header_t* _ak_mem_rem_block(
    _ak_mem_block_header_t* p_block);

void ak_mem_init(void) {
    _ak_mem_avail_size = AK_CFG_HEAP_SIZE;
    _ak_mem_first_block_p = &_ak_mem_initial_block;
    _ak_mem_last_block_p = &_ak_mem_initial_block;
    _ak_mem_start_p = &_ak_mem_arr;
}

void* ak_mem_alloc(size_t size) {
    size = AK_ALIGN_CEIL(size);
    if (!size || _ak_mem_avail_size < size) {
        return NULL;
    }

    _ak_mem_block_header_t* p_block = _ak_mem_find_fit(size);
    if (p_block) {
        size_t block_busy_size = AK_BLOCK_HEADER_SIZE + size;
        size_t block_free_size = p_block->size - block_busy_size;
        _ak_mem_avail_size -= block_busy_size;
        p_block->state = AK_BLOCK_BUSY;

        if (block_free_size >= AK_BLOCK_HEADER_SIZE) {
            _ak_mem_avail_size -= AK_BLOCK_HEADER_SIZE;
            p_block->size = block_busy_size;

            _ak_mem_block_header_t* p_new_block =
                (_ak_mem_block_header_t*)((size_t)p_block + block_busy_size);
            p_new_block->state = AK_BLOCK_FREE;
            p_new_block->size = block_free_size;
            ak_list_insert_after(&p_block->node, &p_new_block->node, 0);
            if (!p_new_block->node.next) {
                _ak_mem_last_block_p = p_new_block;
            }
        }
    }
    return p_block;
}

int ak_mem_free(void* addr) {
    if (!addr || !AK_IS_ALIGNED(addr)) {
        return -1;
    }

    int ret = 0;
    _ak_mem_block_header_t* p_block = _ak_mem_get_block(addr);
    if (!p_block || p_block->state == AK_BLOCK_FREE) {
        ret = -1;
    } else {
        size_t block_net_size = p_block->size - AK_BLOCK_HEADER_SIZE;
        _ak_mem_avail_size += block_net_size;
        p_block->state = AK_BLOCK_FREE;
#ifdef AF_CFG_HEAP_CLEAR_ON_FREE
        memset((void*)((size_t)p_block + AK_BLOCK_HEADER_SIZE), 0,
               block_net_size);
#endif /* AF_CFG_HEAP_CLEAR_ON_FREE */

        _ak_mem_block_header_t* p_prev_block = p_block->node.prev;
        _ak_mem_block_header_t* p_next_block = p_block->node.next;
        if (p_next_block && p_next_block->state == AK_BLOCK_FREE) {
            _ak_mem_rem_block(p_next_block);
        }
        if (p_prev_block && p_prev_block->state == AK_BLOCK_FREE) {
            _ak_mem_rem_block(p_block);
        }
    }
    return ret;
}

size_t ak_mem_get_avail_size(void) {
    return _ak_mem_avail_size;
}

_ak_mem_block_header_t* _ak_mem_find_fit(size_t size) {
    _ak_mem_block_header_t* p = _ak_mem_first_block_p;
    for (; p; p = (_ak_mem_block_header_t*)p->node.next) {
        if (p->state == AK_BLOCK_FREE &&
            p->size - AK_BLOCK_HEADER_SIZE >= size) {
            break;
        }
    }
    return p;
}

_ak_mem_block_header_t* _ak_mem_get_block(void* addr) {
    _ak_mem_block_header_t* p = _ak_mem_first_block_p;
    for (; p; p = (_ak_mem_block_header_t*)p->node.next) {
        if ((size_t)p + AK_BLOCK_HEADER_SIZE == (size_t)addr) {
            break;
        }
    }
    return p;
}

_ak_mem_block_header_t* _ak_mem_rem_block(_ak_mem_block_header_t* p_block) {
    _ak_mem_block_header_t* p_prev_block =
        (_ak_mem_block_header_t*)p_block->node.prev;
    _ak_mem_avail_size += AK_BLOCK_HEADER_SIZE;
    p_prev_block->size += p_block->size;
    ak_list_rm(&p_block->node, 0);
#ifdef AK_CFG_HEAP_CLEAR_ON_FREE
    memset(p_block, 0, AK_BLOCK_HEADER_SIZE);
#endif /* AF_CFG_HEAP_CLEAR_ON_FREE */
    return p_prev_block;
}
