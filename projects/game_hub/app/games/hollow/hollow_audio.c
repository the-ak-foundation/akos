/**
 ******************************************************************************
 * @file    hollow_audio.c
 * @brief   Hollow audio module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "hollow_audio.h"

#include "audio.h"

#include <stddef.h>

#include "buzzer_music.h"
#include "hollow_game.h"

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/

static audio_player_t hollow_audio_player;
static uint32_t hollow_audio_previous_ms;

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void hollow_audio_init(void) {
    audio_player_init(&hollow_audio_player);
    hollow_audio_previous_ms = 0u;
}

void hollow_audio_stop(void) {
    audio_stop(&hollow_audio_player);
}

void hollow_audio_play_events(uint8_t events, uint32_t now) {
    const Tone_TypeDef *sequence = NULL;

    /* Important sounds replace short movement sounds. */
    if ((events & HOLLOW_EVENT_GAME_OVER) != 0u) {
        sequence = tones_lowscore;
    } else if ((events & HOLLOW_EVENT_CRUSH) != 0u) {
        sequence = tones_bang;
    } else if ((events & HOLLOW_EVENT_START) != 0u) {
        sequence = tones_startup;
    } else if ((events & HOLLOW_EVENT_MOVE) != 0u) {
        sequence = tones_click;
    }
    if (sequence == NULL) {
        return;
    }
    hollow_audio_previous_ms = now;
    audio_play_sequence(&hollow_audio_player, sequence);
}

void hollow_audio_update(uint32_t now) {
    uint32_t elapsed_ms = now - hollow_audio_previous_ms;

    hollow_audio_previous_ms = now;
    audio_update(&hollow_audio_player, elapsed_ms);
}
