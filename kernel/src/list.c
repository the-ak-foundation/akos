/**
 * @file list.c
 * @brief Doubly-linked list primitives used by scheduler and timers.
 */

#include "config.h"
#include "list.h"

/**
 * @brief Initialize list sentinel and metadata.
 * @param p_list List object.
 */
void akos_list_init(list_t *const p_list)
{
    p_list->end_item.value = OS_CFG_DELAY_MAX;

    /* End item points to itself when list is empty */
    p_list->end_item.next_ptr = (list_item_t *)&(p_list->end_item);
    p_list->end_item.prev_ptr = (list_item_t *)&(p_list->end_item);

    p_list->num_of_items = (uint16_t)0U;
}

/**
 * @brief Initialize list item ownership metadata.
 * @param p_list_item List item.
 */
void akos_list_item_init(list_item_t *const p_list_item)
{
    p_list_item->list_ptr = NULL; /* The list is more important, all the members of item will be set later */
}

/**
 * @brief Insert item at end of list.
 * @param p_list Destination list.
 * @param p_list_item Item to insert.
 */
void akos_list_insert_end(list_t *const p_list, list_item_t *const p_list_item)
{
    if (list_get_num_item(p_list) == 0u)
    {
        p_list_item->next_ptr = &(p_list->end_item);
        p_list_item->prev_ptr = &(p_list->end_item);

        p_list->end_item.next_ptr = p_list_item;
        p_list->end_item.prev_ptr = p_list_item;

        p_list->curr_item_ptr = p_list_item;
    }
    else
    {
        list_item_t *const p_last_item = p_list->end_item.prev_ptr;

        p_last_item->next_ptr = p_list_item;
        p_list_item->prev_ptr = p_last_item;

        p_list_item->next_ptr = &(p_list->end_item);
        p_list->end_item.prev_ptr = p_list_item;
    }

    /* Remember which list the item is in. */
    p_list_item->list_ptr = p_list;

    (p_list->num_of_items)++;
}

/**
 * @brief Get next item, skipping list end marker.
 * @param p_list_item Current list item.
 * @return Next item in list.
 */
list_item_t *akos_list_item_get_next(list_item_t *p_list_item)
{
    if ((p_list_item)->next_ptr != &(list_item_get_list_contain(p_list_item)->end_item))
        return (p_list_item)->next_ptr;
    else
        return (p_list_item)->next_ptr->next_ptr;
}
/**
 * @brief Get previous item, skipping list end marker.
 * @param p_list_item Current list item.
 * @return Previous item in list.
 */
list_item_t *akos_list_item_get_prev(list_item_t *p_list_item)
{
    if ((p_list_item)->prev_ptr != &(list_item_get_list_contain(p_list_item)->end_item))
        return (p_list_item)->prev_ptr;
    else
        return (p_list_item)->prev_ptr->prev_ptr;
}

/**
 * @brief Insert item by ascending item value.
 * @param p_list Destination list.
 * @param p_list_item Item to insert.
 */
void akos_list_insert(list_t *const p_list, list_item_t *const p_list_item)
{
    if (list_get_num_item(p_list) == 0u)
        akos_list_insert_end(p_list, p_list_item);
    else
    {
        list_item_t *p_iterator;
        const uint32_t value_to_insert = p_list_item->value;
        if (value_to_insert == OS_CFG_DELAY_MAX)
        {
            p_iterator = p_list->end_item.prev_ptr;
        }
        else
        {
            for (p_iterator = (list_item_t *)&(p_list->end_item); p_iterator->next_ptr->value <= value_to_insert; p_iterator = p_iterator->next_ptr)
            {
                /* Iterate through list */
            }
        }
        p_list_item->next_ptr = p_iterator->next_ptr;
        p_list_item->next_ptr->prev_ptr = p_list_item;
        p_list_item->prev_ptr = p_iterator;
        p_iterator->next_ptr = p_list_item;

        /* Remember which list the item is in.  This allows fast removal of the
         * item later. */
        p_list_item->list_ptr = p_list;

        (p_list->num_of_items)++;
    }
}

/**
 * @brief Remove item from its containing list.
 * @param p_list_item Item to remove.
 * @return Remaining number of items in the list.
 */
uint16_t akos_list_remove(list_item_t *const p_list_item)
{
    list_t *const p_list = p_list_item->list_ptr;

    p_list_item->next_ptr->prev_ptr = p_list_item->prev_ptr;
    p_list_item->prev_ptr->next_ptr = p_list_item->next_ptr;

    /* Update curr item to the previous */
    if (p_list->curr_item_ptr == p_list_item)
    {
        p_list->curr_item_ptr = p_list_item->prev_ptr;
    }
    else
    {
        // mtCOVERAGE_TEST_MARKER();
    }

    p_list_item->list_ptr = NULL;
    (p_list->num_of_items)--;

    return p_list->num_of_items;
}

/**
 * @brief Round-robin helper to get owner of next list item.
 * @param p_list List object.
 * @return Owner pointer of next item.
 */
void *akos_list_get_owner_of_next_item(list_t *const p_list)
{
    list_t *const p_const_list = (p_list);
    /* Increment the index to the next item and return the item, ensuring */
    /* we don't return the marker used at the end of the list.  */
    (p_const_list)->curr_item_ptr = (p_const_list)->curr_item_ptr->next_ptr;

    if ((void *)(p_const_list)->curr_item_ptr == (void *)&((p_const_list)->end_item))
    {
        (p_const_list)->curr_item_ptr = (p_const_list)->curr_item_ptr->next_ptr;
    }

    return (p_const_list)->curr_item_ptr->owner_ptr;
}
