#include "ak_list.h"

#include <stddef.h>
#include <stdint.h>

void* ak_list_insert_before(ak_list_node_t* pos,
                            ak_list_node_t* new_node,
                            size_t offset) {
#ifndef NDEBUG
    if (!pos || !new_node) {
        return NULL;
    }
#endif /* NDEBUG */

    void* new_wrapper = (uint8_t*)new_node - offset;
    new_node->next = (uint8_t*)pos - offset;
    new_node->prev = pos->prev;
    if (pos->prev) {
        FWD_OFFSET(ak_list_node_t*, pos->prev, offset)->next = new_wrapper;
    }
    pos->prev = new_wrapper;
    return new_wrapper;
}

void* ak_list_insert_after(ak_list_node_t* pos,
                           ak_list_node_t* new_node,
                           size_t offset) {
#ifndef NDEBUG
    if (!pos || !new_node) {
        return NULL;
    }
#endif /* NDEBUG */

    void* new_wrapper = (uint8_t*)new_node - offset;
    new_node->prev = (uint8_t*)pos - offset;
    new_node->next = pos->next;
    if (pos->next) {
        FWD_OFFSET(ak_list_node_t*, pos->next, offset)->prev = new_wrapper;
    }
    pos->next = new_wrapper;
    return new_wrapper;
}

void* ak_list_rm(ak_list_node_t* pos, size_t offset) {
#ifndef NDEBUG
    if (!pos) {
        return NULL;
    }
#endif /* NDEBUG */

    void* res = NULL;
    void* wrapper = (uint8_t*)pos - offset;
    if (pos->prev != wrapper) {
        res = pos->next;
        if (pos->prev) {
            FWD_OFFSET(ak_list_node_t*, pos->prev, offset)->next = pos->next;
        }
        if (pos->next) {
            FWD_OFFSET(ak_list_node_t*, pos->next, offset)->prev = pos->prev;
        }
    }
    pos->prev = pos->next = NULL;
    return res;
}
