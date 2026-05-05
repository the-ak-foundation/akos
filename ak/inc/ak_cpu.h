#ifndef AK_CPU_H
#define AK_CPU_H

#include "port.h"

#if defined(PORT_GET_IRQ_STATE) && defined(PORT_DIS_IRQ) &&                    \
  defined(PORT_SET_IRQ_STATE)

#  define AK_CPU_CRIT_ENTER()                                                  \
    uint32_t irq_state = PORT_GET_IRQ_STATE();                                 \
    PORT_DIS_IRQ();                                                            \
    {
#  define AK_CPU_CRIT_EXIT()                                                   \
    }                                                                          \
    PORT_SET_IRQ_STATE(irq_state)

#else
#  define AK_CPU_CRIT_ENTER()
#  define AK_CPU_CRIT_EXIT()
#endif /* defined(PORT_GET_IRQ_STATE) && defined(PORT_DIS_IRQ) &&              \
          defined(PORT_SET_IRQ_STATE) */

#endif /* AK_CPU_H */
