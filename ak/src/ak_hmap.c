#include "ak_hmap.h"
#include "ak_list.h"

#include <stdint.h>

int ak_hmap_insert(ak_hmap_t* hmap, int key, void* item) {
    if (!hmap || !hmap->tbl || !hmap->tbl_size) {
        return -1;
    }
    if (!item) {
        return 0;
    }

    size_t bucket = key % hmap->tbl_size;
    void** tbl_entry = &hmap->tbl[bucket];
    void** ends_entry = &hmap->ends[bucket];
    if (!(*tbl_entry)) {
        *tbl_entry = *ends_entry = item;
    } else {
        *ends_entry = ak_list_insert_after(*ends_entry, item, hmap->offset);
    }
    return *tbl_entry ? 0 : -1;
}

int ak_hmap_remove(ak_hmap_t* hmap, int key, void* item) {
    if (!hmap || !hmap->tbl || !hmap->tbl_size) {
        return -1;
    }
    if (!item) {
        return 0;
    }

    size_t bucket = key % hmap->tbl_size;
    void** tbl_entry = &hmap->tbl[bucket];
    void** ends_entry = &hmap->ends[bucket];
    int ret;

    if (!(*tbl_entry)) {
        ret = -1;

    } else {
        ret = 0;

        ak_list_node_t* node = FWD_OFFSET(ak_list_node_t*, item, hmap->offset);
        if (*ends_entry == item) {
            *ends_entry = node->prev;
        }
        if (*tbl_entry == item) {
            *tbl_entry = node->next;
        }
        ak_list_rm(node, hmap->offset);
    }

    return ret;
}
