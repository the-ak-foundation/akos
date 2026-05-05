#ifndef AK_LIST_H
#define AK_LIST_H

#include <stddef.h>

#define FWD_OFFSET(type, ptr, offset) ((type)((uint8_t *)(ptr) + (offset)))

typedef struct _ak_list_node ak_list_node_t;
struct _ak_list_node {
  void *prev, *next;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Insert new_node before pos.
 * @return new_node's wrapper if success. NULL if not.
 */
void *
ak_list_ins_bef(ak_list_node_t *pos, ak_list_node_t *new_node, size_t offset);

/** Insert new_node after pos.
 * @return new_node's wrapper if success. NULL if not.
 */
void *
ak_list_ins_aft(ak_list_node_t *pos, ak_list_node_t *new_node, size_t offset);

/** Remove node at pos.
 * @return pos->next's wrapper if success. NULL if not.
 */
void *ak_list_rm(ak_list_node_t *pos, size_t offset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AK_LIST_H */
