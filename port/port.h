#ifndef PORT_H
#define PORT_H

#if defined(__arm__) || defined(__aarch64__)
    #include "arm/port.h"
#endif  // defined(__arm__) || defined(__aarch64__)

#endif  // PORT_H
