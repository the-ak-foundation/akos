#ifndef AK_MEM_H
#define AK_MEM_H

#include "ak_cfg.h"

#include <stddef.h>

#define AK_ALIGN_CEIL(ptr) \
    (((size_t)(ptr) + AK_CFG_HEAP_ALIGN - 1) & ~(AK_CFG_HEAP_ALIGN - 1))
#define AK_IS_ALIGNED(ptr) (!((size_t)(ptr) & (AK_CFG_HEAP_ALIGN - 1)))

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void ak_mem_init(void);
void* ak_mem_alloc(size_t size);
int ak_mem_free(void* addr);
size_t ak_mem_get_avail_size(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AK_MEM_H */
