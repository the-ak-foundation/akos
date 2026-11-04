/**
 ******************************************************************************
 * @file    buzzer.c
 * @brief   TIM3 PWM buzzer driver
 *
 * @module  Game Hub - Buzzer
 ******************************************************************************
 */

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include "buzzer.h"

#include "io_cfg.h"

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/
#define BUZZER_SYSTEM_CLOCK_HZ (32000000u)
#define BUZZER_DUTY_DIVISOR    (4u)

/*==================================================================================================
*                                         GLOBAL FUNCTIONS
==================================================================================================*/
void buzzer_init(void) {
    uint32_t shift = BUZZER_IO_PIN * 2u;

    RCC->AHBENR |= BUZZER_IO_CLOCK;
    RCC->APB1ENR |= BUZZER_TIM_CLOCK;

    BUZZER_IO_PORT->MODER = (BUZZER_IO_PORT->MODER & ~(3u << shift)) | (2u << shift);
    BUZZER_IO_PORT->OTYPER &= ~(1u << BUZZER_IO_PIN);
    BUZZER_IO_PORT->PUPDR &= ~(3u << shift);
    BUZZER_IO_PORT->AFR[0] = (BUZZER_IO_PORT->AFR[0] & ~(0xFu << (BUZZER_IO_PIN * 4u))) |
                             (BUZZER_IO_AF << (BUZZER_IO_PIN * 4u));

    BUZZER_TIM->CR1 = 0u;
    BUZZER_TIM->PSC = 31u;
    BUZZER_TIM->CCMR2 = (6u << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;
    BUZZER_TIM->CCER = TIM_CCER_CC3E;
    BUZZER_TIM->EGR = TIM_EGR_UG;
}

void buzzer_tone(uint16_t frequency_hz) {
    uint32_t timer_hz = BUZZER_SYSTEM_CLOCK_HZ / 32u;
    uint32_t period;

    if (frequency_hz == 0u) {
        buzzer_off();
        return;
    }

    period = timer_hz / frequency_hz;
    if (period < 2u) {
        period = 2u;
    }

    BUZZER_TIM->ARR = period - 1u;
    /* 25% duty: half the drive time of the previous 50% square wave. */
    BUZZER_TIM->CCR3 = period / BUZZER_DUTY_DIVISOR;
    BUZZER_TIM->EGR = TIM_EGR_UG;
    BUZZER_TIM->CR1 |= TIM_CR1_CEN;
}

void buzzer_off(void) {
    BUZZER_TIM->CR1 &= ~TIM_CR1_CEN;
    BUZZER_TIM->CCR3 = 0u;
}
