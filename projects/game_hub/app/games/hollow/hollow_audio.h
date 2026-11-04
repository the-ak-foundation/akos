/**
 ******************************************************************************
 * @file    hollow_audio.h
 * @brief   Hollow audio module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __HOLLOW_AUDIO_H__
#define __HOLLOW_AUDIO_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void hollow_audio_init(void);
void hollow_audio_stop(void);
void hollow_audio_play_events(uint8_t events, uint32_t now);
void hollow_audio_update(uint32_t now);

#ifdef __cplusplus
}
#endif

#endif /* __HOLLOW_AUDIO_H__ */
