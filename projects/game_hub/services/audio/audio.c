/**
 ******************************************************************************
 * @file    audio.c
 * @brief   Shared nonblocking tone and sequence playback
 *
 * @module  Game Hub - Audio
 ******************************************************************************
 */

#include "audio.h"

#include <stddef.h>

#include "audio_port.h"
#include "buzzer.h"

static audio_player_t *audio_owner;

static void audio_reset_player(audio_player_t *player) {
    player->sequence = NULL;
    player->remaining_ms = 0u;
    player->continuous = false;
}

static void audio_stop_locked(audio_player_t *player) {
    if (audio_owner == player) {
        audio_owner = NULL;
        buzzer_off();
    }
    audio_reset_player(player);
}

static void audio_set_frequency(uint16_t frequency_hz) {
    if (frequency_hz == 0u) {
        buzzer_off();
    } else {
        buzzer_tone(frequency_hz);
    }
}

void audio_player_init(audio_player_t *player) {
    audio_stop(player);
}

void audio_stop(audio_player_t *player) {
    uint32_t state;

    if (player == NULL) {
        return;
    }
    state = audio_port_lock();
    audio_stop_locked(player);
    audio_port_unlock(state);
}

void audio_stop_all(void) {
    uint32_t state = audio_port_lock();

    /* Do not dereference the previous caller's storage. */
    audio_owner = NULL;
    buzzer_off();
    audio_port_unlock(state);
}

void audio_play_tone(audio_player_t *player, uint16_t frequency_hz, uint32_t duration_ms) {
    uint32_t state;

    if (player == NULL) {
        return;
    }
    state = audio_port_lock();
    audio_reset_player(player);
    player->remaining_ms = duration_ms;
    player->continuous = duration_ms == 0u;
    audio_owner = player;
    audio_set_frequency(frequency_hz);
    audio_port_unlock(state);
}

void audio_play_sequence(audio_player_t *player, const Tone_TypeDef *sequence) {
    uint32_t state;

    if (player == NULL) {
        return;
    }
    state = audio_port_lock();
    audio_stop_locked(player);
    if ((sequence != NULL) && (sequence->duration != 0u)) {
        player->sequence = sequence;
        player->remaining_ms = (uint32_t)sequence->duration * 10u;
        audio_owner = player;
        audio_set_frequency(sequence->frequency);
    }
    audio_port_unlock(state);
}

void audio_update(audio_player_t *player, uint32_t elapsed_ms) {
    uint32_t state;

    if (player == NULL) {
        return;
    }
    state = audio_port_lock();
    if (audio_owner != player) {
        audio_reset_player(player);
    } else if (!player->continuous) {
        while (elapsed_ms >= player->remaining_ms) {
            elapsed_ms -= player->remaining_ms;
            if (player->sequence == NULL) {
                audio_stop_locked(player);
                break;
            }
            player->sequence++;
            if (player->sequence->duration == 0u) {
                audio_stop_locked(player);
                break;
            }
            player->remaining_ms = (uint32_t)player->sequence->duration * 10u;
            audio_set_frequency(player->sequence->frequency);
        }
        if (audio_owner == player) {
            player->remaining_ms -= elapsed_ms;
        }
    }
    audio_port_unlock(state);
}
