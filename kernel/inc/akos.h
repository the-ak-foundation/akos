/**
  ******************************************************************************
  * @file    akos.h
  * @brief   Single public include for AKOS applications.
  *
  * @author  HUYNNG | BUUPQ
  * @date    Created: 2026-06-11
  * @date    Updated: 2026-06-25
  * 
  * @module  AKOS
  ******************************************************************************
  */

#ifndef __AKOS_H__
#define __AKOS_H__

/* Version -------------------------------------------------------------------*/
#define AKOS_VERSION_MAIN 1u
#define AKOS_VERSION_SUB1 0u
#define AKOS_VERSION_SUB2 0u

#define AKOS_VERSION                                                          \
    ((AKOS_VERSION_MAIN << 16u) | (AKOS_VERSION_SUB1 << 8u) |                 \
     AKOS_VERSION_SUB2)

/* Includes ------------------------------------------------------------------*/
#include "core.h"
#include "thread.h"
#include "timer.h"
#include "message.h"

#endif /* __AKOS_H__ */
