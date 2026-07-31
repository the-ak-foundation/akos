#include "math_rush_game.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "app.h"
#include "game_gfx.h"
#include "game_hal.h"

#define MATH_RUSH_READY_TIME_MS    (1000u)
#define MATH_RUSH_FEEDBACK_TIME_MS (600u)
#define MATH_RUSH_LEVEL_TIME_MS    (800u)
#define MATH_RUSH_SOUND_TIME_MS    (180u)
#define MATH_RUSH_QUESTIONS_LEVEL  (10u)

static uint32_t math_rush_high_score;

static uint32_t math_rush_question_time(uint8_t level) {
    if (level == 1u) {
        return 5000u;
    }
    if (level == 2u) {
        return 4500u;
    }
    if (level == 3u) {
        return 4000u;
    }
    if (level == 4u) {
        return 3500u;
    }
    return 3000u;
}

static void math_rush_begin_question(math_rush_game_t *game) {
    math_rush_generate_question(game->level, &game->question);
    game->question_time_ms = math_rush_question_time(game->level);
    game->remaining_time_ms = game->question_time_ms;
    game->selected_time_ms = 0u;
    game->selected_answer = MATH_ANSWER_LEFT;
    game->answer_selected = false;
    game->state_time_ms = 0u;
    game->state = MATH_RUSH_STATE_QUESTION;
}

static void math_rush_finish_question(math_rush_game_t *game, bool correct) {
    game->questions_in_level++;
    game->state_time_ms = 0u;
    if (correct) {
        uint32_t points;

        if (game->combo < UINT8_MAX) {
            game->combo++;
        }
        points = math_rush_calculate_score(game->level, game->combo,
                                           game->selected_time_ms);
        game->score = (UINT32_MAX - game->score < points) ?
                      UINT32_MAX : game->score + points;
        game->state = MATH_RUSH_STATE_CORRECT;
        game_buzzer_tone(2400u);
    } else {
        game->combo = 0u;
        if (game->lives > 0u) {
            game->lives--;
        }
        game->state = MATH_RUSH_STATE_WRONG;
        game_buzzer_tone(500u);
    }
    game->feedback_sound_on = true;
}

static void math_rush_advance(math_rush_game_t *game) {
    if (game->lives == 0u) {
        if (game->score > math_rush_high_score) {
            math_rush_high_score = game->score;
        }
        game->high_score = math_rush_high_score;
        game->state = MATH_RUSH_STATE_GAME_OVER;
        game->state_time_ms = 0u;
    } else if (game->questions_in_level >= MATH_RUSH_QUESTIONS_LEVEL) {
        if (game->level < UINT8_MAX) {
            game->level++;
        }
        game->questions_in_level = 0u;
        game->state = MATH_RUSH_STATE_LEVEL_UP;
        game->state_time_ms = 0u;
    } else {
        math_rush_begin_question(game);
    }
}

void math_rush_reset(math_rush_game_t *game, uint32_t seed) {
    if (game == NULL) {
        return;
    }
    memset(game, 0, sizeof(*game));
    math_rush_question_seed(seed ^ 0x4D415448u);
    game->high_score = math_rush_high_score;
    game->level = 1u;
    game->lives = 3u;
    game->state = MATH_RUSH_STATE_READY;
}

void math_rush_game_init(void *context, uint32_t seed) {
    math_rush_reset((math_rush_game_t *)context, seed);
}

void math_rush_game_deinit(void *context) {
    math_rush_game_t *game = (math_rush_game_t *)context;

    if (game != NULL) {
        game->remaining_time_ms = 0u;
        game->input_previous = 0u;
    }
    game_buzzer_off();
}

void math_rush_game_update(void *context, uint8_t input_state,
                           uint32_t elapsed_ms) {
    math_rush_game_t *game = (math_rush_game_t *)context;
    uint8_t pressed;

    if (game == NULL) {
        return;
    }
    pressed = (uint8_t)(input_state & ~game->input_previous);
    game->input_previous = input_state;

    switch (game->state) {
    case MATH_RUSH_STATE_READY:
        game->state_time_ms += elapsed_ms;
        if (game->state_time_ms >= MATH_RUSH_READY_TIME_MS) {
            math_rush_begin_question(game);
        }
        break;

    case MATH_RUSH_STATE_QUESTION:
        /* Swap answer buttons only inside Math Rush; menu stays unchanged. */
        if ((pressed & GAME_INPUT_DOWN) != 0u) {
            game->selected_answer = MATH_ANSWER_LEFT;
            game->answer_selected = true;
        } else if ((pressed & GAME_INPUT_UP) != 0u) {
            game->selected_answer = MATH_ANSWER_RIGHT;
            game->answer_selected = true;
        }
        if (((pressed & GAME_INPUT_MODE) != 0u) && game->answer_selected) {
            /* MODE confirms immediately. The remaining time is the bonus. */
            game->selected_time_ms = game->remaining_time_ms;
            math_rush_finish_question(
                game, math_rush_check_answer(&game->question,
                                              game->selected_answer));
            break;
        }
        if (elapsed_ms >= game->remaining_time_ms) {
            game->remaining_time_ms = 0u;
            game->selected_time_ms = 0u;
            math_rush_finish_question(
                game, game->answer_selected &&
                      math_rush_check_answer(&game->question,
                                             game->selected_answer));
        } else {
            game->remaining_time_ms -= elapsed_ms;
        }
        break;

    case MATH_RUSH_STATE_CORRECT:
    case MATH_RUSH_STATE_WRONG:
        game->state_time_ms += elapsed_ms;
        if (game->feedback_sound_on &&
            (game->state_time_ms >= MATH_RUSH_SOUND_TIME_MS)) {
            game_buzzer_off();
            game->feedback_sound_on = false;
        }
        if (game->state_time_ms >= MATH_RUSH_FEEDBACK_TIME_MS) {
            math_rush_advance(game);
        }
        break;

    case MATH_RUSH_STATE_LEVEL_UP:
        game->state_time_ms += elapsed_ms;
        if (game->state_time_ms >= MATH_RUSH_LEVEL_TIME_MS) {
            math_rush_begin_question(game);
        }
        break;

    case MATH_RUSH_STATE_GAME_OVER:
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game->return_to_menu = true;
        } else if ((pressed & GAME_INPUT_UP) != 0u) {
            math_rush_reset(game, game->score ^ game->state_time_ms);
        } else {
            game->state_time_ms += elapsed_ms;
        }
        break;

    default:
        math_rush_reset(game, 1u);
        break;
    }
}

static char *math_rush_append_number(char *output, uint32_t value) {
    char reverse[10];
    uint8_t count = 0u;

    do {
        reverse[count++] = (char)('0' + value % 10u);
        value /= 10u;
    } while ((value > 0u) && (count < sizeof(reverse)));
    while (count > 0u) {
        *output++ = reverse[--count];
    }
    return output;
}

static char *math_rush_append_signed(char *output, int32_t value) {
    if (value < 0) {
        *output++ = '-';
        value = -value;
    }
    return math_rush_append_number(output, (uint32_t)value);
}

static char math_rush_operator_char(math_operator_t operation) {
    static const char symbols[] = {'+', '-', 'x', '/'};

    return (operation <= MATH_OPERATOR_DIVIDE) ? symbols[operation] : '?';
}

static void math_rush_draw_header(const math_rush_game_t *game) {
    char text[28];
    char *cursor = text;

    *cursor++ = 'L';
    *cursor++ = 'V';
    *cursor++ = ':';
    cursor = math_rush_append_number(cursor, game->level);
    *cursor++ = ' ';
    *cursor++ = 'S';
    *cursor++ = ':';
    cursor = math_rush_append_number(cursor, game->score);
    *cursor++ = ' ';
    *cursor++ = 'L';
    *cursor++ = ':';
    cursor = math_rush_append_number(cursor, game->lives);
    *cursor = '\0';
    game_gfx_draw_text(1u, 1u, text, 1u, true, false);
}

static void math_rush_draw_question(const math_rush_game_t *game) {
    char text[24];
    char *cursor = text;
    uint8_t length;
    uint8_t x;
    uint8_t size;

    if (game->question.has_second_operation) {
        *cursor++ = '(';
    }
    cursor = math_rush_append_signed(cursor, game->question.left_operand);
    *cursor++ = ' ';
    *cursor++ = math_rush_operator_char(game->question.operation);
    *cursor++ = ' ';
    cursor = math_rush_append_signed(cursor, game->question.right_operand);
    if (game->question.has_second_operation) {
        *cursor++ = ')';
        *cursor++ = ' ';
        *cursor++ = math_rush_operator_char(
            game->question.second_operation);
        *cursor++ = ' ';
        cursor = math_rush_append_signed(cursor,
                                         game->question.third_operand);
    }
    *cursor = '\0';
    length = (uint8_t)(cursor - text);
    size = (length <= 10u) ? 2u : 1u;
    x = (uint8_t)((GAME_DISPLAY_WIDTH -
                   (uint16_t)length * 6u * size) / 2u);
    game_gfx_draw_text(x, (size == 2u) ? 13u : 18u, text, size, true, false);
}

static void math_rush_draw_answer(uint8_t x, int32_t answer, bool selected) {
    char text[6];
    char *end = math_rush_append_signed(text, answer);
    uint8_t digits = (uint8_t)(end - text);
    uint8_t size = (digits <= 4u) ? 2u : 1u;
    uint8_t text_x = (uint8_t)(x +
        (52u - (uint16_t)digits * 6u * size) / 2u);

    *end = '\0';

    if (selected) {
        game_display_fill_rect(x, 33u, 52u, 19u, true);
    } else {
        game_gfx_draw_rect(x, 33u, 52u, 19u, true);
    }
    game_gfx_draw_text(text_x, (size == 2u) ? 35u : 39u, text, size,
                       !selected, selected);
}

static void math_rush_draw_time(const math_rush_game_t *game) {
    uint32_t width = 0u;

    game_gfx_draw_text(1u, 56u, "T", 1u, true, false);
    game_gfx_draw_rect(9u, 56u, 118u, 8u, true);
    if (game->question_time_ms > 0u) {
        width = (game->remaining_time_ms * 114u) / game->question_time_ms;
    }
    if (width > 0u) {
        game_display_fill_rect(11u, 58u, (uint8_t)width, 4u, true);
    }
}

void math_rush_game_draw(const void *context) {
    const math_rush_game_t *game = (const math_rush_game_t *)context;

    if (game == NULL) {
        return;
    }
    game_display_clear();
    math_rush_draw_header(game);

    if (game->state == MATH_RUSH_STATE_READY) {
        game_gfx_draw_text(16u, 20u, "MATH RUSH", 2u, true, false);
        game_gfx_draw_text(34u, 44u, "GET READY", 1u, true, false);
        return;
    }
    if (game->state == MATH_RUSH_STATE_LEVEL_UP) {
        game_gfx_draw_text(16u, 20u, "LEVEL UP", 2u, true, false);
        return;
    }
    if (game->state == MATH_RUSH_STATE_GAME_OVER) {
        game_gfx_draw_text(10u, 14u, "GAME OVER", 2u, true, false);
        game_gfx_draw_text(15u, 36u, "HI:", 1u, true, false);
        game_gfx_draw_number(33u, 36u, game->high_score, 1u, true, false);
        game_gfx_draw_text(15u, 48u, "UP:RETRY MODE:MENU", 1u,
                           true, false);
        return;
    }

    math_rush_draw_question(game);
    math_rush_draw_answer(
        5u, game->question.answers[MATH_ANSWER_LEFT],
        game->answer_selected && game->selected_answer == MATH_ANSWER_LEFT);
    math_rush_draw_answer(
        71u, game->question.answers[MATH_ANSWER_RIGHT],
        game->answer_selected && game->selected_answer == MATH_ANSWER_RIGHT);
    if (game->state == MATH_RUSH_STATE_CORRECT) {
        game_display_fill_rect(14u, 13u, 100u, 18u, false);
        game_gfx_draw_text(16u, 14u, "CORRECT!", 2u, true, false);
    } else if (game->state == MATH_RUSH_STATE_WRONG) {
        game_display_fill_rect(26u, 13u, 76u, 18u, false);
        game_gfx_draw_text(28u, 14u, "WRONG!", 2u, true, false);
    }
    math_rush_draw_time(game);
}
