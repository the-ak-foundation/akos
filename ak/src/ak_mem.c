#include "ak_mem.h"
#include "ak_cfg.h"
#include "ak_list.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef AK_CFG_HEAP_SZ
#  define AK_MEM_SZ AK_CFG_HEAP_SZ
#else
#  define AK_MEM_SZ 0x200u
#endif /* AK_CFG_HEAP_SZ */

#ifdef AK_CFG_HEAP_ALIGN
#  define AK_MEM_ALIGN AK_CFG_HEAP_ALIGN
#else
#  define AK_MEM_ALIGN 4u
#endif /* AK_CFG_HEAP_ALIGN */

#ifdef AF_CFG_HEAP_CLEAR_ON_FREE
#  define AK_MEM_CLEAR_ON_FREE AF_CFG_HEAP_CLEAR_ON_FREE
#else
#  define AK_MEM_CLEAR_ON_FREE AF_CFG_HEAP_CLEAR_ON_FREE 0u
#endif /* AF_CFG_HEAP_CLEAR_ON_FREE */

#define AK_ALIGN_CEIL(ptr)                                                     \
  (((size_t)(ptr) + AK_MEM_ALIGN - 1) & ~(AK_MEM_ALIGN - 1))
#define AK_IS_ALIGNED(ptr) (!((size_t)(ptr) & (AK_MEM_ALIGN - 1)))
#define AK_BLK_HDR_SZ      AK_ALIGN_CEIL(sizeof(_ak_mem_blk_hdr_t))

typedef enum : uint8_t { AK_BLK_FREE, AK_BLK_BUSY } _ak_mem_blk_state_t;

typedef struct {
  ak_list_node_t node;
  size_t sz; /* size including header */
  _ak_mem_blk_state_t state;
} _ak_mem_blk_hdr_t;

static size_t _ak_mem_avail_sz;
static _ak_mem_blk_hdr_t *_ak_mem_first_blk_p;
static _ak_mem_blk_hdr_t *_ak_mem_last_blk_p;
static void *_ak_mem_start_p;

/* These two must stick together. */
static _ak_mem_blk_hdr_t
  __attribute__((aligned(AK_MEM_ALIGN))) _ak_mem_ini_blk = {
    .node = {.prev = NULL, .next = NULL},
    .sz = AK_BLK_HDR_SZ + AK_MEM_SZ,
    .state = AK_BLK_FREE
};
static uint8_t
  __attribute__((aligned(AK_MEM_ALIGN))) _ak_mem_arr[AK_CFG_HEAP_SZ];

static void _ak_mem_init(void);
static inline _ak_mem_blk_hdr_t *_ak_mem_find_fit(size_t sz);
static inline _ak_mem_blk_hdr_t *_ak_mem_get_blk(void *addr);
static inline _ak_mem_blk_hdr_t *_ak_mem_rem_blk(_ak_mem_blk_hdr_t *p_blk);

void *ak_mem_alloc(size_t sz) {
  if (!_ak_mem_start_p) {
    _ak_mem_init();
  }

  sz = AK_ALIGN_CEIL(sz);
  if (!sz || _ak_mem_avail_sz < sz) {
    return NULL;
  }

  _ak_mem_blk_hdr_t *p_blk = _ak_mem_find_fit(sz);
  if (p_blk) {
    size_t blk_busy_sz = AK_BLK_HDR_SZ + sz;
    size_t blk_free_sz = p_blk->sz - blk_busy_sz;
    _ak_mem_avail_sz -= blk_busy_sz;
    p_blk->state = AK_BLK_BUSY;

    if (blk_free_sz >= AK_BLK_HDR_SZ) {
      _ak_mem_avail_sz -= AK_BLK_HDR_SZ;
      p_blk->sz = blk_busy_sz;

      _ak_mem_blk_hdr_t *p_new_blk =
        (_ak_mem_blk_hdr_t *)((size_t)p_blk + blk_busy_sz);
      p_new_blk->state = AK_BLK_FREE;
      p_new_blk->sz = blk_free_sz;
      ak_list_ins_aft(&p_blk->node, &p_new_blk->node, 0);
      if (!p_new_blk->node.next) {
        _ak_mem_last_blk_p = p_new_blk;
      }
    }
  }
  return p_blk;
}

int ak_mem_free(void *addr) {
  if (!_ak_mem_start_p) {
    _ak_mem_init();
    return -1;
  }

  if (!addr || !AK_IS_ALIGNED(addr)) {
    return -1;
  }

  int ret = 0;
  _ak_mem_blk_hdr_t *p_blk = _ak_mem_get_blk(addr);
  if (!p_blk || p_blk->state == AK_BLK_FREE) {
    ret = -1;
  } else {
    size_t blk_net_sz = p_blk->sz - AK_BLK_HDR_SZ;
    _ak_mem_avail_sz += blk_net_sz;
    p_blk->state = AK_BLK_FREE;
#ifdef AK_MEM_CLEAR_ON_FREE
    memset((void *)((size_t)p_blk + AK_BLK_HDR_SZ), 0, blk_net_sz);
#endif /* AK_MEM_CLEAR_ON_FREE */

    _ak_mem_blk_hdr_t *p_prev_blk = (_ak_mem_blk_hdr_t *)p_blk->node.prev;
    _ak_mem_blk_hdr_t *p_next_blk = (_ak_mem_blk_hdr_t *)p_blk->node.next;
    if (p_next_blk && p_next_blk->state == AK_BLK_FREE) {
      _ak_mem_rem_blk(p_next_blk);
    }
    if (p_prev_blk && p_prev_blk->state == AK_BLK_FREE) {
      _ak_mem_rem_blk(p_blk);
    }
  }
  return ret;
}

size_t ak_mem_get_avail_sz(void) { return _ak_mem_avail_sz; }

void _ak_mem_init(void) {
  _ak_mem_avail_sz = AK_MEM_SZ;
  _ak_mem_first_blk_p = &_ak_mem_ini_blk;
  _ak_mem_last_blk_p = &_ak_mem_ini_blk;
#ifdef __GNUC__
  _ak_mem_start_p = &_ak_mem_arr;
#else
  _ak_mem_start_p = (void *)AK_ALIGN_CEIL(&_ak_mem_arr);
#endif /* __GNUC__ */
}

_ak_mem_blk_hdr_t *_ak_mem_find_fit(size_t sz) {
  _ak_mem_blk_hdr_t *p = _ak_mem_first_blk_p;
  for (; p; p = (_ak_mem_blk_hdr_t *)p->node.next) {
    if (p->state == AK_BLK_FREE && p->sz - AK_BLK_HDR_SZ >= sz) {
      break;
    }
  }
  return p;
}

_ak_mem_blk_hdr_t *_ak_mem_get_blk(void *addr) {
  _ak_mem_blk_hdr_t *p = _ak_mem_first_blk_p;
  for (; p; p = (_ak_mem_blk_hdr_t *)p->node.next) {
    if ((size_t)p + AK_BLK_HDR_SZ == (size_t)addr) {
      break;
    }
  }
  return p;
}

_ak_mem_blk_hdr_t *_ak_mem_rem_blk(_ak_mem_blk_hdr_t *p_blk) {
  _ak_mem_blk_hdr_t *p_prev_blk = (_ak_mem_blk_hdr_t *)p_blk->node.prev;
  _ak_mem_avail_sz += AK_BLK_HDR_SZ;
  p_prev_blk->sz += p_blk->sz;
  ak_list_rm(&p_blk->node, 0);
#ifdef AK_MEM_CLEAR_ON_FREE
  memset(p_blk, 0, AK_BLK_HDR_SZ);
#endif /* AK_MEM_CLEAR_ON_FREE */
  return p_prev_blk;
}
