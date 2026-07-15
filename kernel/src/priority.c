/**
 ******************************************************************************
 * @file    priority.c
 * @brief   Priority bitmap operations.
 *
 * @author  Snoopy3921 - AK Foundation
 * @date    Created: 2026/05/08
 * @date    Updated: 2026-06-26
 *
 * @module  AKOS
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "priority.h"

/* Private variables ---------------------------------------------------------*/
static uint32_t prio_curr;
static uint8_t prio_tbl[OS_PRIO_TBL_SIZE];

/* Function definitions ------------------------------------------------------*/
/**
 * @brief Initialize ready-priority bitmap table.
 */
void akos_priority_init(void) {
    uint8_t i;

    for (i = 0u; i < OS_PRIO_TBL_SIZE; i++) {
        prio_tbl[i] = 0u;
    }

    /* OS_CFG_PRIO_MAX-1 is the lowest priority level and that is idle task's prio. */
    akos_priority_insert(OS_CFG_PRIO_MAX - 1);
}

/**
 * @brief Insert priority into ready bitmap.
 * @param prio Priority value.
 */
void akos_priority_insert(uint32_t prio) {
    uint8_t bit;
    uint8_t row;

    row = (uint32_t)(prio / (8u));
    bit = (uint8_t)prio & ((8u) - 1u);
    prio_tbl[row] |= (uint8_t)1u << (((8u) - 1u) - bit);
}

/**
 * @brief Remove priority from ready bitmap.
 * @param prio Priority value.
 */
void akos_priority_remove(uint32_t prio) {
    uint8_t bit;
    uint8_t row;

    row = (uint32_t)(prio / (8u));
    bit = (uint8_t)prio & ((8u) - 1u);
    prio_tbl[row] &= ~((uint8_t)1u << (((8u) - 1u) - bit));
}

/**
 * @brief Get highest ready priority.
 * @return Highest ready priority value.
 */
uint32_t akos_priority_get_highest(void) {
    uint8_t* p_tbl;
    uint32_t prio;

    prio  = 0u;
    p_tbl = &prio_tbl[0];
    while (*p_tbl == 0u) {
        /* Search the bitmap table for the highest priority     */
        /* Compute the step of each CPU_DATA entry              */
        prio += (8u);
        p_tbl++;
    }

    uint8_t bit = (uint8_t)prio & (8u - 1u);
    while (!(*p_tbl & ((uint8_t)1u << ((8u - 1u) - bit)))) {
        prio++;
        bit = (uint8_t)prio & (8u - 1u);
    }
    return (prio);
}

/**
 * @brief Get current scheduler priority tracker.
 * @return Current priority value.
 */
uint32_t akos_priority_get_current(void) {
    return prio_curr;
}
