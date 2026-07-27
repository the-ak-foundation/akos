#ifndef __MATH_RUSH_GAME_H__
#define __MATH_RUSH_GAME_H__

#include <stdbool.h>
#include <stdint.h>

#include "math_rush_question.h"

typedef enum {
    MATH_RUSH_STATE_READY = 0,
    MATH_RUSH_STATE_QUESTION,
    MATH_RUSH_STATE_CORRECT,
    MATH_RUSH_STATE_WRONG,
    MATH_RUSH_STATE_LEVEL_UP,
    MATH_RUSH_STATE_GAME_OVER,
} math_rush_state_t;

typedef struct {
    math_question_t question;
    uint32_t score;
    uint32_t high_score;
    uint32_t state_time_ms;
    uint32_t question_time_ms;
    uint32_t remaining_time_ms;
    uint32_t selected_time_ms;
    uint8_t level;
    uint8_t lives;
    uint8_t combo;
    uint8_t questions_in_level;
    uint8_t input_previous;
    math_answer_position_t selected_answer;
    bool answer_selected;
    bool feedback_sound_on;
    bool return_to_menu;
    math_rush_state_t state;
} math_rush_game_t;

void math_rush_reset(math_rush_game_t *game, uint32_t seed);
void math_rush_game_init(void *context, uint32_t seed);
void math_rush_game_deinit(void *context);
void math_rush_game_update(void *context, uint8_t input_state,
                           uint32_t elapsed_ms);
void math_rush_game_draw(const void *context);

#endif /* __MATH_RUSH_GAME_H__ */
