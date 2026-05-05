#ifndef AK_MEM_H
#define AK_MEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void *ak_mem_alloc(size_t sz);
int ak_mem_free(void *addr);
size_t ak_mem_get_avail_sz(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AK_MEM_H */
