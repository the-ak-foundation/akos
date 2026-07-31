#ifndef __BIKE_RACER_GAME_H__
#define __BIKE_RACER_GAME_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BIKE_RACER_READY = 0,
    BIKE_RACER_RACING,
    BIKE_RACER_GAME_OVER,
    BIKE_RACER_FINISHED,
} bike_racer_state_t;

typedef struct {
    uint32_t elapsed_ms;
    uint32_t state_ms;
    uint32_t distance;
    uint16_t update_accumulator_ms;
    uint16_t sound_timer_ms;
    int16_t player_x;
    int16_t road_center;
    int16_t curve;
    int16_t drift_accumulator;
    int8_t lean;
    uint8_t speed;
    uint8_t lap;
    uint8_t input_previous;
    bool sound_on;
    bool return_to_menu;
    bike_racer_state_t state;
} bike_racer_game_t;

void bike_racer_game_init(void *context, uint32_t seed);
void bike_racer_game_deinit(void *context);
void bike_racer_game_update(void *context, uint8_t input_state,
                            uint32_t elapsed_ms);
void bike_racer_game_draw(const void *context);

#endif /* __BIKE_RACER_GAME_H__ */
