#ifndef AK_HMAP_H
#define AK_HMAP_H

#include "ak_list.h"

/* array of lists */
typedef struct {
    void **tbl, **ends;
    size_t tbl_size, offset;
} ak_hmap_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int ak_hmap_insert(ak_hmap_t* hmap, int key, void* item);
int ak_hmap_remove(ak_hmap_t* hmap, int key, void* item);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AK_HMAP_H */
