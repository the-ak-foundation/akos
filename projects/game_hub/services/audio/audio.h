/**
 ******************************************************************************
 * @file    audio.h
 * @brief   Shared nonblocking tone and sequence playback
 *
 * @module  Game Hub - Audio
 ******************************************************************************
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdint.h>

#include "buzzer_music.h"

#ifdef __cplusplus
extern "C" {
#endif

/* One player per caller. Only the most recently played sound owns the buzzer. */
typedef struct {
    const Tone_TypeDef *sequence;
    uint32_t remaining_ms;
    bool continuous;
} audio_player_t;

/* Initialization is safe before the player storage has been initialized. */
void audio_player_init(audio_player_t *player);
void audio_stop(audio_player_t *player);
void audio_stop_all(void);
/* A zero duration holds the tone until stopped or replaced. */
void audio_play_tone(audio_player_t *player, uint16_t frequency_hz, uint32_t duration_ms);
/* Sequence storage must remain valid until stopped. A zero-duration note ends it. */
void audio_play_sequence(audio_player_t *player, const Tone_TypeDef *sequence);
/* Advance this player's clock once per elapsed interval; preempted sounds never resume. */
void audio_update(audio_player_t *player, uint32_t elapsed_ms);

#ifdef __cplusplus
}
#endif

#endif
