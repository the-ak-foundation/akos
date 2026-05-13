#ifndef PORT_ARM_V6M_H
#define PORT_ARM_V6M_H

__attribute__((naked)) void PendSV_Handler(void);
__attribute__((naked)) void SVC_Handler(void);

#endif  // PORT_ARM_V6M_H
