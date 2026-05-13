#include <stdint.h>

#include "ak_hashmap.h"
#include "ak_list.h"

int ak_hashmap_insert(ak_hashmap_t* hashmap, int key, void* item) {
#ifndef NDEBUG
    if (!hashmap || !hashmap->table || !hashmap->table_size) {
        return -1;
    }
    if (!item) {
        return 0;
    }
#endif  // NDEBUG

    size_t bucket = key % hashmap->table_size;
    void** table_entry = &hashmap->table[bucket];
    if (!(*table_entry)) {
        *table_entry = item;
    } else {
        ak_list_node_t* entry_node =
            FWD_OFFSET(ak_list_node_t*, table_entry, hashmap->offset);
        ak_list_node_t* item_node =
            FWD_OFFSET(ak_list_node_t*, item, hashmap->offset);
        *table_entry =
            ak_list_insert_before(entry_node, item_node, hashmap->offset);
    }
    return *table_entry ? 0 : -1;
}

int ak_hashmap_remove(ak_hashmap_t* hashmap, int key, void* item) {
#ifndef NDEBUG
    if (!hashmap || !hashmap->table || !hashmap->table_size) {
        return -1;
    }
    if (!item) {
        return 0;
    }
#endif  // NDEBUG

    size_t bucket = key % hashmap->table_size;
    void** table_entry = &hashmap->table[bucket];
    int ret;

    if (!(*table_entry)) {
        ret = -1;

    } else {
        ret = 0;

        ak_list_node_t* node =
            FWD_OFFSET(ak_list_node_t*, item, hashmap->offset);
        void* next_wrapper = ak_list_remove(node, hashmap->offset);
        if (*table_entry == item) {
            *table_entry = next_wrapper;
        }
    }

    return ret;
}
