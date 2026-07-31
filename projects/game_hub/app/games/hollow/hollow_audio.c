#include "hollow_audio.h"

#include <stddef.h>

#include "buzzer_music.h"
#include "game_hal.h"
#include "hollow_game.h"

static const Tone_TypeDef *hollow_audio_sequence;
static uint8_t hollow_audio_index;
static uint32_t hollow_audio_deadline;

static void hollow_audio_start_tone(uint32_t now) {
    const Tone_TypeDef *tone;

    if (hollow_audio_sequence == NULL) {
        game_buzzer_off();
        return;
    }
    tone = &hollow_audio_sequence[hollow_audio_index];
    if (tone->duration == 0u) {
        hollow_audio_sequence = NULL;
        game_buzzer_off();
        return;
    }
    if (tone->frequency == 0u) {
        game_buzzer_off();
    } else {
        game_buzzer_tone(tone->frequency);
    }
    hollow_audio_deadline = now + (uint32_t)tone->duration * 10u;
}

void hollow_audio_init(void) {
    hollow_audio_sequence = NULL;
    hollow_audio_index = 0u;
    hollow_audio_deadline = 0u;
    game_buzzer_init();
    game_buzzer_off();
}

void hollow_audio_stop(void) {
    hollow_audio_sequence = NULL;
    hollow_audio_index = 0u;
    hollow_audio_deadline = 0u;
    game_buzzer_off();
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
    hollow_audio_sequence = sequence;
    hollow_audio_index = 0u;
    hollow_audio_start_tone(now);
}

void hollow_audio_update(uint32_t now) {
    if ((hollow_audio_sequence == NULL) ||
        ((int32_t)(now - hollow_audio_deadline) < 0)) {
        return;
    }
    hollow_audio_index++;
    hollow_audio_start_tone(now);
}
