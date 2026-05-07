/*
 * priority.h
 *
 *  Created on: Jun 26, 2024
 *      Author: giahu
 */

#ifndef PRIORITY_H
#define PRIORITY_H

/**
 * @file priority.h
 * @brief Priority bitmap scheduler helpers.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "config.h"

#define OS_PRIO_TBL_SIZE (((OS_CFG_PRIO_MAX - 1u) / (8u)) + 1u)

    /**
     * @brief Initialize priority table.
     */
    void akos_priority_init(void);
    /**
     * @brief Mark priority as ready.
     * @param prio Priority value.
     */
    void akos_priority_insert(uint32_t prio);
    /**
     * @brief Clear priority from ready table.
     * @param prio Priority value.
     */
    void akos_priority_remove(uint32_t prio);
    /**
     * @brief Get highest ready priority.
     * @return Highest ready priority value.
     */
    uint32_t akos_priority_get_highest(void);
    /**
     * @brief Get current priority tracker.
     * @return Current priority value.
     */
    uint32_t akos_priority_get_current(void);

#ifdef __cplusplus
}
#endif
#endif /* PRIORITY_H */
