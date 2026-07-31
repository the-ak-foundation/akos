#ifndef __MATH_RUSH_QUESTION_H__
#define __MATH_RUSH_QUESTION_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MATH_OPERATOR_ADD = 0,
    MATH_OPERATOR_SUBTRACT,
    MATH_OPERATOR_MULTIPLY,
    MATH_OPERATOR_DIVIDE,
} math_operator_t;

typedef enum {
    MATH_ANSWER_LEFT = 0,
    MATH_ANSWER_RIGHT,
    MATH_ANSWER_COUNT,
} math_answer_position_t;

typedef struct {
    int32_t left_operand;
    int32_t right_operand;
    int32_t third_operand;
    int32_t answers[MATH_ANSWER_COUNT];
    int32_t correct_answer;
    math_operator_t operation;
    math_operator_t second_operation;
    math_answer_position_t correct_position;
    bool has_second_operation;
} math_question_t;

void math_rush_question_seed(uint32_t seed);
void math_rush_generate_question(uint8_t level, math_question_t *question);
bool math_rush_check_answer(const math_question_t *question,
                            math_answer_position_t selected);
int32_t math_rush_create_wrong_answer(int32_t correct, math_operator_t op,
                                      uint8_t level);
uint32_t math_rush_calculate_score(uint8_t level, uint8_t combo,
                                   uint32_t remaining_time_ms);

#endif /* __MATH_RUSH_QUESTION_H__ */
