#include "ak_mem.h"
#include "ak_cfg.h"
#include "ak_list.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define AK_ALIGN_CEIL(ptr) \
    (((size_t)(ptr) + AK_CFG_HEAP_ALIGN - 1) & ~(AK_CFG_HEAP_ALIGN - 1))
#define AK_IS_ALIGNED(ptr) (!((size_t)(ptr) & (AK_CFG_HEAP_ALIGN - 1)))
#define AK_BLK_HDR_size    AK_ALIGN_CEIL(sizeof(_ak_mem_blk_hdr_t))

typedef enum : uint8_t { AK_BLK_FREE, AK_BLK_BUSY } _ak_mem_blk_state_t;

typedef struct {
    ak_list_node_t node;
    size_t size; /* size including header */
    _ak_mem_blk_state_t state;
} _ak_mem_blk_hdr_t;

static size_t _ak_mem_avail_size;
static _ak_mem_blk_hdr_t* _ak_mem_first_blk_p;
static _ak_mem_blk_hdr_t* _ak_mem_last_blk_p;
static void* _ak_mem_start_p;

/* These two must stick together. */
static _ak_mem_blk_hdr_t
    __attribute__((aligned(AK_CFG_HEAP_ALIGN))) _ak_mem_ini_blk = {
        .node = {.prev = NULL, .next = NULL},
        .size = AK_BLK_HDR_size + AK_CFG_HEAP_size,
        .state = AK_BLK_FREE};
static uint8_t
    __attribute__((aligned(AK_CFG_HEAP_ALIGN))) _ak_mem_arr[AK_CFG_HEAP_size];

static void _ak_mem_init(void);
static inline _ak_mem_blk_hdr_t* _ak_mem_find_fit(size_t size);
static inline _ak_mem_blk_hdr_t* _ak_mem_get_blk(void* addr);
static inline _ak_mem_blk_hdr_t* _ak_mem_rem_blk(_ak_mem_blk_hdr_t* p_blk);

void* ak_mem_alloc(size_t size) {
    if (!_ak_mem_start_p) {
        _ak_mem_init();
    }

    size = AK_ALIGN_CEIL(size);
    if (!size || _ak_mem_avail_size < size) {
        return NULL;
    }

    _ak_mem_blk_hdr_t* p_blk = _ak_mem_find_fit(size);
    if (p_blk) {
        size_t blk_busy_size = AK_BLK_HDR_size + size;
        size_t blk_free_size = p_blk->size - blk_busy_size;
        _ak_mem_avail_size -= blk_busy_size;
        p_blk->state = AK_BLK_BUSY;

        if (blk_free_size >= AK_BLK_HDR_size) {
            _ak_mem_avail_size -= AK_BLK_HDR_size;
            p_blk->size = blk_busy_size;

            _ak_mem_blk_hdr_t* p_new_blk =
                (_ak_mem_blk_hdr_t*)((size_t)p_blk + blk_busy_size);
            p_new_blk->state = AK_BLK_FREE;
            p_new_blk->size = blk_free_size;
            ak_list_insert_after(&p_blk->node, &p_new_blk->node, 0);
            if (!p_new_blk->node.next) {
                _ak_mem_last_blk_p = p_new_blk;
            }
        }
    }
    return p_blk;
}

int ak_mem_free(void* addr) {
    if (!_ak_mem_start_p) {
        _ak_mem_init();
        return -1;
    }

    if (!addr || !AK_IS_ALIGNED(addr)) {
        return -1;
    }

    int ret = 0;
    _ak_mem_blk_hdr_t* p_blk = _ak_mem_get_blk(addr);
    if (!p_blk || p_blk->state == AK_BLK_FREE) {
        ret = -1;
    } else {
        size_t blk_net_size = p_blk->size - AK_BLK_HDR_size;
        _ak_mem_avail_size += blk_net_size;
        p_blk->state = AK_BLK_FREE;
#ifdef AF_CFG_HEAP_CLEAR_ON_FREE
        memset((void*)((size_t)p_blk + AK_BLK_HDR_size), 0, blk_net_size);
#endif /* AF_CFG_HEAP_CLEAR_ON_FREE */

        _ak_mem_blk_hdr_t* p_prev_blk = (_ak_mem_blk_hdr_t*)p_blk->node.prev;
        _ak_mem_blk_hdr_t* p_next_blk = (_ak_mem_blk_hdr_t*)p_blk->node.next;
        if (p_next_blk && p_next_blk->state == AK_BLK_FREE) {
            _ak_mem_rem_blk(p_next_blk);
        }
        if (p_prev_blk && p_prev_blk->state == AK_BLK_FREE) {
            _ak_mem_rem_blk(p_blk);
        }
    }
    return ret;
}

size_t ak_mem_get_avail_size(void) {
    return _ak_mem_avail_size;
}

void _ak_mem_init(void) {
    _ak_mem_avail_size = AK_CFG_HEAP_size;
    _ak_mem_first_blk_p = &_ak_mem_ini_blk;
    _ak_mem_last_blk_p = &_ak_mem_ini_blk;
    _ak_mem_start_p = &_ak_mem_arr;
}

_ak_mem_blk_hdr_t* _ak_mem_find_fit(size_t size) {
    _ak_mem_blk_hdr_t* p = _ak_mem_first_blk_p;
    for (; p; p = (_ak_mem_blk_hdr_t*)p->node.next) {
        if (p->state == AK_BLK_FREE && p->size - AK_BLK_HDR_size >= size) {
            break;
        }
    }
    return p;
}

_ak_mem_blk_hdr_t* _ak_mem_get_blk(void* addr) {
    _ak_mem_blk_hdr_t* p = _ak_mem_first_blk_p;
    for (; p; p = (_ak_mem_blk_hdr_t*)p->node.next) {
        if ((size_t)p + AK_BLK_HDR_size == (size_t)addr) {
            break;
        }
    }
    return p;
}

_ak_mem_blk_hdr_t* _ak_mem_rem_blk(_ak_mem_blk_hdr_t* p_blk) {
    _ak_mem_blk_hdr_t* p_prev_blk = (_ak_mem_blk_hdr_t*)p_blk->node.prev;
    _ak_mem_avail_size += AK_BLK_HDR_size;
    p_prev_blk->size += p_blk->size;
    ak_list_rm(&p_blk->node, 0);
#ifdef AF_CFG_HEAP_CLEAR_ON_FREE
    memset(p_blk, 0, AK_BLK_HDR_size);
#endif /* AF_CFG_HEAP_CLEAR_ON_FREE */
    return p_prev_blk;
}
