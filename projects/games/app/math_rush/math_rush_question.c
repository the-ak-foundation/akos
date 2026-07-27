#include "math_rush_question.h"

#include <limits.h>
#include <stddef.h>

static uint32_t math_random_state = 0x4D415448u;

static uint32_t math_random(void) {
    uint32_t value = math_random_state;

    value ^= value << 13u;
    value ^= value >> 17u;
    value ^= value << 5u;
    math_random_state = (value == 0u) ? 0x4D415448u : value;
    return math_random_state;
}

static uint32_t math_random_range(uint32_t maximum) {
    return (maximum == 0u) ? 0u : math_random() % maximum;
}

static int32_t math_apply_operation(int32_t left, int32_t right,
                                    math_operator_t operation) {
    switch (operation) {
    case MATH_OPERATOR_ADD:
        return left + right;
    case MATH_OPERATOR_SUBTRACT:
        return left - right;
    case MATH_OPERATOR_MULTIPLY:
        return left * right;
    case MATH_OPERATOR_DIVIDE:
        return (right == 0) ? 0 : left / right;
    default:
        return 0;
    }
}

static void math_use_nonzero_unit_operation(
    int32_t result, math_operator_t *operation, int32_t *operand) {
    if (result > 999) {
        *operation = MATH_OPERATOR_SUBTRACT;
        *operand = result - 999;
    } else if (result < -999) {
        *operation = MATH_OPERATOR_ADD;
        *operand = -999 - result;
    } else {
        *operation = (result > -999) ? MATH_OPERATOR_SUBTRACT :
                                      MATH_OPERATOR_ADD;
        *operand = 1;
    }
}

void math_rush_question_seed(uint32_t seed) {
    math_random_state = (seed == 0u) ? 0x4D415448u : seed;
}

int32_t math_rush_create_wrong_answer(int32_t correct, math_operator_t op,
                                      uint8_t level) {
    static const int16_t close_offsets[] = {-2, -1, 1, 2};
    static const int16_t medium_offsets[] = {-10, -3, -2, -1,
                                              1,   2,  3, 10};
    static const int16_t hard_offsets[] = {-100, -10, -2, -1,
                                              1,   2, 10, 100};
    const int16_t *offsets = close_offsets;
    uint8_t offset_count = 4u;
    int32_t wrong;
    uint8_t index;

    if ((op == MATH_OPERATOR_MULTIPLY) ||
        (op == MATH_OPERATOR_DIVIDE) || (level >= 3u)) {
        offsets = medium_offsets;
        offset_count = 8u;
    }
    if (level >= 8u) {
        offsets = hard_offsets;
        offset_count = 8u;
    }
    index = (uint8_t)math_random_range(offset_count);
    wrong = correct + offsets[index];
    if (wrong < -999) {
        wrong = correct + 1;
    } else if (wrong > 999) {
        wrong = correct - 1;
    }
    if (wrong == correct) {
        wrong++;
    }
    return wrong;
}

void math_rush_generate_question(uint8_t level, math_question_t *question) {
    uint32_t range;
    int32_t left;
    int32_t right;
    int32_t third = 0;
    int32_t result;
    math_operator_t operation;
    math_operator_t second_operation = MATH_OPERATOR_ADD;
    bool has_second_operation = level >= 5u;

    if (question == NULL) {
        return;
    }
    if (level == 0u) {
        level = 1u;
    }

    if (level == 1u) {
        operation = MATH_OPERATOR_ADD;
        range = 10u;
    } else if (level == 2u) {
        operation = (math_operator_t)math_random_range(2u);
        range = 31u;
    } else if (level == 3u) {
        operation = (math_operator_t)math_random_range(2u);
        range = 100u;
    } else if (level == 4u) {
        operation = MATH_OPERATOR_MULTIPLY;
        range = 8u;
    } else {
        operation = (math_operator_t)math_random_range(4u);
        range = 200u + ((uint32_t)level - 5u) * 200u;
        if (range > 1000u) {
            range = 1000u;
        }
    }

    if (operation == MATH_OPERATOR_MULTIPLY) {
        uint32_t maximum = 9u;

        if (level == 5u) {
            maximum = 12u;
        } else if (level == 6u) {
            maximum = 15u;
        } else if (level == 7u) {
            maximum = 18u;
        } else if (level >= 8u) {
            maximum = 20u;
        }

        left = 2 + (int32_t)math_random_range(maximum - 1u);
        right = 2 + (int32_t)math_random_range(maximum - 1u);
        result = left * right;
    } else if (operation == MATH_OPERATOR_DIVIDE) {
        uint32_t maximum = 12u + ((uint32_t)level - 5u) * 3u;

        if (maximum > 20u) {
            maximum = 20u;
        }
        right = 2 + (int32_t)math_random_range(maximum - 1u);
        result = 1 + (int32_t)math_random_range(range / (uint32_t)right);
        left = result * right;
    } else {
        uint32_t minimum = (level == 3u) ? 10u : 1u;

        left = (int32_t)(minimum + math_random_range(range - minimum));
        right = (int32_t)(minimum + math_random_range(range - minimum));
        if ((level <= 3u) && (operation == MATH_OPERATOR_SUBTRACT) &&
            (right > left)) {
            int32_t swap = left;
            left = right;
            right = swap;
        }
        result = math_apply_operation(left, right, operation);
    }

    if (has_second_operation) {
        uint32_t factor_maximum = 12u + ((uint32_t)level - 5u) * 3u;

        if (factor_maximum > 20u) {
            factor_maximum = 20u;
        }
        second_operation = (math_operator_t)math_random_range(4u);
        if (second_operation == MATH_OPERATOR_ADD) {
            int32_t maximum = 999 - result;

            if (maximum < 1) {
                math_use_nonzero_unit_operation(
                    result, &second_operation, &third);
            } else {
                if (maximum >= (int32_t)range) {
                    maximum = (int32_t)range - 1;
                }
                third = 1 + (int32_t)math_random_range((uint32_t)maximum);
            }
        }
        if (second_operation == MATH_OPERATOR_SUBTRACT) {
            int32_t minimum = result - 999;
            int32_t maximum = result + 999;

            if (minimum < 1) {
                minimum = 1;
            }
            if (maximum >= (int32_t)range) {
                maximum = (int32_t)range - 1;
            }
            if (maximum < minimum) {
                math_use_nonzero_unit_operation(
                    result, &second_operation, &third);
            } else {
                third = minimum + (int32_t)math_random_range(
                    (uint32_t)(maximum - minimum + 1));
            }
        } else if (second_operation == MATH_OPERATOR_MULTIPLY) {
            int32_t absolute = (result < 0) ? -result : result;
            uint32_t allowed_maximum = factor_maximum;

            if (absolute > 0) {
                uint32_t result_limit = 999u / (uint32_t)absolute;
                if (allowed_maximum > result_limit) {
                    allowed_maximum = result_limit;
                }
            }
            if (allowed_maximum < 2u) {
                math_use_nonzero_unit_operation(
                    result, &second_operation, &third);
            } else {
                third = 2 + (int32_t)math_random_range(allowed_maximum - 1u);
            }
        } else if (second_operation == MATH_OPERATOR_DIVIDE) {
            uint32_t divisor_count = 0u;
            int32_t divisors[19];

            for (uint32_t divisor = 2u; divisor <= factor_maximum; divisor++) {
                if ((result != 0) && (result % (int32_t)divisor == 0)) {
                    divisors[divisor_count++] = (int32_t)divisor;
                }
            }
            if (divisor_count == 0u) {
                math_use_nonzero_unit_operation(
                    result, &second_operation, &third);
            } else {
                third = divisors[math_random_range(divisor_count)];
            }
        }
        result = math_apply_operation(result, third, second_operation);
    }

    question->left_operand = left;
    question->right_operand = right;
    question->third_operand = third;
    question->operation = operation;
    question->second_operation = second_operation;
    question->has_second_operation = has_second_operation;
    question->correct_answer = result;
    question->correct_position = (math_answer_position_t)(math_random() & 1u);
    question->answers[question->correct_position] = result;
    question->answers[question->correct_position == MATH_ANSWER_LEFT ?
                      MATH_ANSWER_RIGHT : MATH_ANSWER_LEFT] =
        math_rush_create_wrong_answer(
            result, has_second_operation ? second_operation : operation,
            level);
}

bool math_rush_check_answer(const math_question_t *question,
                            math_answer_position_t selected) {
    if ((question == NULL) || (selected >= MATH_ANSWER_COUNT)) {
        return false;
    }
    return selected == question->correct_position;
}

uint32_t math_rush_calculate_score(uint8_t level, uint8_t combo,
                                   uint32_t remaining_time_ms) {
    uint32_t score = 10u * level;

    if (UINT32_MAX - score < remaining_time_ms / 100u) {
        return UINT32_MAX;
    }
    score += remaining_time_ms / 100u;
    if (UINT32_MAX - score < (uint32_t)combo * 2u) {
        return UINT32_MAX;
    }
    return score + (uint32_t)combo * 2u;
}
