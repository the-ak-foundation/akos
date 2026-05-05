#ifndef AK_PRIO_H
#define AK_PRIO_H

#include <stdint.h>

typedef int ak_prio_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void ak_prio_reset(void);

/* prio must start from 1 */
int ak_prio_bit_set(ak_prio_t prio);
int ak_prio_bit_clear(ak_prio_t prio);

/*
 * @return Highest set priority. -1 if none is found.
 */
ak_prio_t ak_prio_get_top(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AK_PRIO_H */
