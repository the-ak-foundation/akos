/**
 ******************************************************************************
 * @file    buzzer.h
 * @brief   PWM buzzer driver API
 *
 * @module  Game Hub - Buzzer
 ******************************************************************************
 */

#ifndef __BUZZER_H__
#define __BUZZER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void buzzer_init(void);
void buzzer_tone(uint16_t frequency_hz);
void buzzer_off(void);

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H__ */
