/*
 * list.h
 *
 *  Created on: Jun 26, 2024
 *      Author: giahu
 */
/**
 * @file list.h
 * @brief Doubly-linked list types and APIs for scheduler internals.
 */

#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "core.h"

	/** @brief List container type. */
	typedef struct list list_t;
	/** @brief List node type. */
	typedef struct list_item list_item_t;

	/**
	 * @struct list_item
	 * @brief Doubly-linked list node used by scheduler/timer lists.
	 */
	struct list_item
	{
		struct list_item *next_ptr; /**< Pointer to next item. */
		struct list_item *prev_ptr; /**< Pointer to previous item. */
		uint32_t value;             /**< Sort key used by ordered insert. */
		void *owner_ptr;            /**< Owner object (usually TCB or timer). */
		struct list *list_ptr;      /**< List currently containing this item, or NULL. */
	};

	/**
	 * @struct list
	 * @brief Doubly-linked list metadata with end sentinel.
	 */
	struct list
	{
		struct list_item *curr_item_ptr; /**< Iterator cursor for round-robin traversal. */
		struct list_item end_item;       /**< End sentinel item. */
		uint16_t num_of_items;           /**< Number of real items (excluding sentinel). */
	};

#define list_item_set_owner(p_list_item, p_owner) ((p_list_item)->owner_ptr = (void *)(p_owner))
#define list_item_get_owner(p_list_item) ((p_list_item)->owner_ptr)
#define list_item_set_value(p_list_item, val) ((p_list_item)->value = (uint32_t)(val))
#define list_item_get_value(p_list_item) ((p_list_item)->value)
#define list_item_get_list_contain(p_list_item) ((p_list_item)->list_ptr)

#define list_get_head_item_value(p_list) ((((p_list)->end_item).next_ptr)->value)
#define list_get_head_item(p_list) (((p_list)->end_item).next_ptr)
#define list_get_owner_of_head_item(p_list) (list_item_get_owner(((p_list)->end_item).next_ptr))
#define list_get_end_item(p_list) ((p_list)->end_item)
#define list_get_num_item(p_list) (((p_list)->num_of_items))
#define list_is_empty(p_list) (((p_list)->num_of_items == (uint8_t)0) ? OS_TRUE : OS_FALSE)

	/**
	 * @brief Get owner of next item and advance list cursor.
	 * @param p_list List pointer.
	 * @return Owner pointer of next item.
	 */
	void *akos_list_get_owner_of_next_item(list_t *const p_list);

	/**
	 * @brief Initialize list metadata and sentinel.
	 * @param p_list List pointer.
	 */
	void akos_list_init(list_t *const p_list);

	/**
	 * @brief Initialize list item linkage and ownership metadata.
	 * @param p_list_item List item pointer.
	 */
	void akos_list_item_init(list_item_t *const p_list_item);

	/**
	 * @brief Get next list item.
	 * @param p_list_item Current item pointer.
	 * @return Next item pointer.
	 */
	list_item_t *akos_list_item_get_next(list_item_t *p_list_item);

	/**
	 * @brief Get previous list item.
	 * @param p_list_item Current item pointer.
	 * @return Previous item pointer.
	 */
	list_item_t *akos_list_item_get_prev(list_item_t *p_list_item);

	/**
	 * @brief Insert item at list tail.
	 * @param p_list List pointer.
	 * @param p_list_item Item pointer.
	 */
	void akos_list_insert_end(list_t *const p_list, list_item_t *const p_list_item);

	/**
	 * @brief Insert item in ascending order by @ref list_item.value.
	 * @param p_list List pointer.
	 * @param p_list_item Item pointer.
	 */
	void akos_list_insert(list_t *const p_list, list_item_t *const p_list_item);

	/**
	 * @brief Remove item from containing list.
	 * @param p_list_item Item pointer.
	 * @return Remaining number of items in the list after removal.
	 */
	uint16_t akos_list_remove(list_item_t *const p_list_item);

#ifdef __cplusplus
}
#endif
#endif /* LIST_H */
