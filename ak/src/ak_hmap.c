#include "ak_hmap.h"
#include "ak_list.h"

#include <stdint.h>

int ak_hmap_ins(ak_hmap_t *hmap, int key, void *item) {
  if (!hmap || !hmap->tbl || !hmap->tbl_sz) {
    return -1;
  }
  if (!item) {
    return 0;
  }

  size_t bucket = key % hmap->tbl_sz;
  void **tbl_ent = &hmap->tbl[bucket];
  void **ends_ent = &hmap->ends[bucket];
  if (!(*tbl_ent)) {
    *tbl_ent = *ends_ent = item;
  } else {
    *ends_ent = ak_list_ins_aft(*ends_ent, item, hmap->offset);
  }
  return *tbl_ent ? 0 : -1;
}

int ak_hmap_rm(ak_hmap_t *hmap, int key, void *item) {
  if (!hmap || !hmap->tbl || !hmap->tbl_sz) {
    return -1;
  }
  if (!item) {
    return 0;
  }

  size_t bucket = key % hmap->tbl_sz;
  void **tbl_ent = &hmap->tbl[bucket];
  void **ends_ent = &hmap->ends[bucket];
  int ret;
  
  if (!(*tbl_ent)) {
    ret = -1;
    
  } else {
    ret = 0;

    ak_list_node_t *node = FWD_OFFSET(ak_list_node_t *, item, hmap->offset);
    if (*ends_ent == item) {
      *ends_ent = node->prev;
    }
    if (*tbl_ent == item) {
      *tbl_ent = node->next;
    }
    ak_list_rm(node, hmap->offset);
  }

  return ret;
}
