/**
 ******************************************************************************
 * @file    audio_port.h
 * @brief   Platform critical section hooks for audio ownership
 *
 * @module  Game Hub - Audio
 ******************************************************************************
 */

#ifndef AUDIO_PORT_H
#define AUDIO_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t audio_port_lock(void);
void audio_port_unlock(uint32_t state);

#ifdef __cplusplus
}
#endif

#endif
