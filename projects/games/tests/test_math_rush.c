#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app.h"
#include "math_rush_game.h"
#include "math_rush_question.h"

static uint16_t buzzer_frequency;

void game_display_clear(void) {
}

void game_display_fill_rect(uint8_t x, uint8_t y, uint8_t width,
                            uint8_t height, bool on) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)on;
}

void game_gfx_draw_text(uint8_t x, uint8_t y, const char *text,
                        uint8_t size, bool foreground, bool background) {
    (void)x;
    (void)y;
    (void)text;
    (void)size;
    (void)foreground;
    (void)background;
}

void game_gfx_draw_number(uint8_t x, uint8_t y, uint32_t value,
                          uint8_t size, bool foreground, bool background) {
    (void)x;
    (void)y;
    (void)value;
    (void)size;
    (void)foreground;
    (void)background;
}

void game_gfx_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                        bool on) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)on;
}

void game_buzzer_off(void) {
    buzzer_frequency = 0u;
}

void game_buzzer_tone(uint16_t frequency_hz) {
    buzzer_frequency = frequency_hz;
}

static uint8_t answer_input(const math_question_t *question) {
    return (question->correct_position == MATH_ANSWER_LEFT) ?
           GAME_INPUT_DOWN : GAME_INPUT_UP;
}

static void release_and_finish_feedback(math_rush_game_t *game) {
    math_rush_game_update(game, 0u, 1u);
    math_rush_game_update(game, 0u, 600u);
}

static int32_t apply_operation(int32_t left, int32_t right,
                               math_operator_t operation) {
    switch (operation) {
    case MATH_OPERATOR_ADD: return left + right;
    case MATH_OPERATOR_SUBTRACT: return left - right;
    case MATH_OPERATOR_MULTIPLY: return left * right;
    case MATH_OPERATOR_DIVIDE: return left / right;
    default: return 0;
    }
}

static void test_question_generator(void) {
    bool saw_left = false;
    bool saw_right = false;
    bool saw_negative = false;
    bool saw_second_operation[4] = {false, false, false, false};

    math_rush_question_seed(0x12345678u);
    for (uint8_t level = 1u; level <= 10u; level++) {
        for (uint16_t index = 0u; index < 1000u; index++) {
            math_question_t question;
            uint8_t correct_count = 0u;
            int32_t intermediate;

            math_rush_generate_question(level, &question);
            if (level == 1u) {
                assert(question.operation == MATH_OPERATOR_ADD);
                assert(question.left_operand <= 9);
                assert(question.right_operand <= 9);
            } else if (level == 2u) {
                assert(question.operation <= MATH_OPERATOR_SUBTRACT);
                assert(question.left_operand <= 30);
                assert(question.right_operand <= 30);
            } else if (level == 3u) {
                assert(question.operation <= MATH_OPERATOR_SUBTRACT);
                assert(question.left_operand >= 10);
                assert(question.right_operand >= 10);
                assert(question.left_operand <= 99);
                assert(question.right_operand <= 99);
            } else if (level == 4u) {
                assert(question.operation == MATH_OPERATOR_MULTIPLY);
                assert(question.left_operand >= 2);
                assert(question.right_operand >= 2);
                assert(question.left_operand <= 9);
                assert(question.right_operand <= 9);
            } else {
                assert(question.has_second_operation);
                assert(question.second_operation <= MATH_OPERATOR_DIVIDE);
                saw_second_operation[question.second_operation] = true;
            }
            assert(question.answers[MATH_ANSWER_LEFT] !=
                   question.answers[MATH_ANSWER_RIGHT]);
            assert(question.answers[MATH_ANSWER_LEFT] >= -999);
            assert(question.answers[MATH_ANSWER_LEFT] <= 999);
            assert(question.answers[MATH_ANSWER_RIGHT] >= -999);
            assert(question.answers[MATH_ANSWER_RIGHT] <= 999);
            assert(question.correct_answer >= -999);
            assert(question.correct_answer <= 999);
            if ((level >= 5u) && (question.correct_answer < 0)) {
                saw_negative = true;
            }
            for (uint8_t answer = 0u; answer < MATH_ANSWER_COUNT; answer++) {
                if (question.answers[answer] == question.correct_answer) {
                    correct_count++;
                }
            }
            assert(correct_count == 1u);
            assert(math_rush_check_answer(&question,
                                           question.correct_position));
            if (question.correct_position == MATH_ANSWER_LEFT) {
                saw_left = true;
            } else {
                saw_right = true;
            }
            if (question.operation == MATH_OPERATOR_DIVIDE) {
                assert(question.right_operand != 0);
                assert(question.left_operand % question.right_operand == 0);
            }
            if ((question.operation == MATH_OPERATOR_ADD) ||
                (question.operation == MATH_OPERATOR_SUBTRACT)) {
                assert(question.left_operand != 0);
                assert(question.right_operand != 0);
            }
            if ((level <= 3u) &&
                (question.operation == MATH_OPERATOR_SUBTRACT)) {
                assert(question.correct_answer >= 0);
            }
            intermediate = apply_operation(question.left_operand,
                                           question.right_operand,
                                           question.operation);
            if (question.has_second_operation) {
                if ((question.second_operation == MATH_OPERATOR_ADD) ||
                    (question.second_operation == MATH_OPERATOR_SUBTRACT)) {
                    assert(question.third_operand != 0);
                }
                if (question.second_operation == MATH_OPERATOR_DIVIDE) {
                    assert(question.third_operand != 0);
                    assert(intermediate % question.third_operand == 0);
                }
                assert(apply_operation(intermediate, question.third_operand,
                                       question.second_operation) ==
                       question.correct_answer);
            } else {
                assert(intermediate == question.correct_answer);
            }
        }
    }
    assert(saw_left && saw_right);
    assert(saw_negative);
    for (uint8_t operation = 0u; operation < 4u; operation++) {
        assert(saw_second_operation[operation]);
    }
}

static void test_state_machine(void) {
    math_rush_game_t game;

    math_rush_game_init(&game, 42u);
    assert(game.state == MATH_RUSH_STATE_READY);
    assert(game.lives == 3u);
    math_rush_game_update(&game, 0u, 1000u);
    assert(game.state == MATH_RUSH_STATE_QUESTION);

    /* MODE without a selected answer must not submit anything. */
    math_rush_game_update(&game, GAME_INPUT_MODE, 16u);
    assert(game.state == MATH_RUSH_STATE_QUESTION);
    assert(!game.answer_selected);
    math_rush_game_update(&game, 0u, 16u);

    for (uint8_t question = 0u; question < 10u; question++) {
        uint32_t score_before = game.score;
        uint32_t expected;

        math_rush_game_update(&game, answer_input(&game.question), 16u);
        assert(game.state == MATH_RUSH_STATE_QUESTION);
        assert(game.answer_selected);
        math_rush_game_update(&game, answer_input(&game.question), 16u);
        assert(game.state == MATH_RUSH_STATE_QUESTION);
        expected = math_rush_calculate_score(
            game.level, (uint8_t)(game.combo + 1u), game.remaining_time_ms);
        math_rush_game_update(&game, GAME_INPUT_MODE, 16u);
        assert(game.state == MATH_RUSH_STATE_CORRECT);
        assert(buzzer_frequency == 2400u);
        assert(game.score == score_before + expected);
        release_and_finish_feedback(&game);
        assert(buzzer_frequency == 0u);
        if (question < 9u) {
            assert(game.state == MATH_RUSH_STATE_QUESTION);
        }
    }
    assert(game.state == MATH_RUSH_STATE_LEVEL_UP);
    assert(game.level == 2u);
    math_rush_game_update(&game, 0u, 800u);
    assert(game.state == MATH_RUSH_STATE_QUESTION);

    game.question.correct_position = MATH_ANSWER_RIGHT;
    math_rush_game_update(&game, GAME_INPUT_DOWN, 16u);
    assert(game.state == MATH_RUSH_STATE_QUESTION);
    assert(game.answer_selected);
    assert(game.selected_answer == MATH_ANSWER_LEFT);
    math_rush_game_update(&game, GAME_INPUT_DOWN, 16u);
    assert(game.lives == 3u);
    math_rush_game_update(&game, GAME_INPUT_MODE, 16u);
    assert(game.state == MATH_RUSH_STATE_WRONG);
    assert(buzzer_frequency == 500u);
    assert(game.lives == 2u);
    release_and_finish_feedback(&game);
    assert(game.state == MATH_RUSH_STATE_QUESTION);

    math_rush_game_update(&game, 0u, game.remaining_time_ms);
    assert(game.state == MATH_RUSH_STATE_WRONG);
    assert(game.lives == 1u);

    math_rush_game_deinit(&game);
    assert(game.remaining_time_ms == 0u);
}

int main(void) {
    test_question_generator();
    test_state_machine();
    puts("math_rush: all tests passed");
    return 0;
}
