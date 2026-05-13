#ifndef AK_HASHMAP_H
#define AK_HASHMAP_H

#include <stddef.h>

typedef struct {
    void** table;  // array of lists
    size_t table_size, offset;
} ak_hashmap_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int ak_hashmap_insert(ak_hashmap_t* hashmap, int key, void* item);
int ak_hashmap_remove(ak_hashmap_t* hashmap, int key, void* item);
void* ak_hashmap_find(ak_hashmap_t* hashmap, int key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AK_HASHMAP_H */
