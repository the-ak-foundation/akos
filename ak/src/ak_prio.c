#include "ak_prio.h"
#include "ak_cfg.h"
#include "port.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if (__SIZEOF_SIZE_T__ <= 4)
#  define CLZ(num) __builtin_clz(num)
#  define CTZ(num) __builtin_ctz(num)
#else
#  define CLZ(num) __builtin_clzll(num)
#  define CTZ(num) __builtin_ctzll(num)
#endif /* __SIZEOF_SIZE_T__ */

#ifdef AK_CFG_PRIO_MAX
#  define AK_PRIO_MAX AK_CFG_PRIO_MAX
#else
#  define AK_PRIO_MAX 255u
#endif /* AK_CFG_PRIO_MAX */

#define AK_NPRIO_PER_ENT      (sizeof(size_t) << 3)
#define AK_NPRIO_PER_ENT_POW2 CTZ(AK_NPRIO_PER_ENT)
#define AK_PRIO_BITSET_SZ                                                      \
  (((AK_PRIO_MAX + AK_NPRIO_PER_ENT - 1) >> AK_NPRIO_PER_ENT_POW2) + 1)

static size_t _ak_prio_bitset[AK_PRIO_BITSET_SZ];

void ak_prio_reset(void) {
  memset(_ak_prio_bitset, 0, sizeof(_ak_prio_bitset));
}

int ak_prio_bit_set(ak_prio_t prio) {
#ifndef NDEBUG
  if (prio < 0 || prio > AK_CFG_PRIO_MAX) {
    return -1;
  }
#endif /* NDEBUG */

  size_t ent = prio >> AK_NPRIO_PER_ENT_POW2;
  size_t offset = prio & (AK_NPRIO_PER_ENT - 1);
  _ak_prio_bitset[ent] |= 1LL << offset;
  return 0;
}

int ak_prio_bit_clear(ak_prio_t prio) {
#ifndef NDEBUG
  if (prio < 0 || prio > AK_CFG_PRIO_MAX) {
    return -1;
  }
#endif /* NDEBUG */

  size_t ent = prio >> AK_NPRIO_PER_ENT_POW2;
  size_t offset = prio & (AK_NPRIO_PER_ENT - 1);
  _ak_prio_bitset[ent] &= ~(1LL << offset);
  return 0;
}

ak_prio_t ak_prio_get_top(void) {
  size_t res = (AK_PRIO_BITSET_SZ << AK_NPRIO_PER_ENT_POW2) - 1;
  for (int i = AK_PRIO_BITSET_SZ - 1; i >= 0; --i) {
    if (_ak_prio_bitset[i]) {
      res -= CLZ(_ak_prio_bitset[i]);
      break;
    }
    res -= AK_NPRIO_PER_ENT;
  }
  return res;
}
