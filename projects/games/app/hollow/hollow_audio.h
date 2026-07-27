#ifndef __HOLLOW_AUDIO_H__
#define __HOLLOW_AUDIO_H__

#include <stdint.h>

void hollow_audio_init(void);
void hollow_audio_stop(void);
void hollow_audio_play_events(uint8_t events, uint32_t now);
void hollow_audio_update(uint32_t now);

#endif /* __HOLLOW_AUDIO_H__ */
