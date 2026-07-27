#ifndef __MAP3D_GAME_H__
#define __MAP3D_GAME_H__

#include <stdbool.h>
#include <stdint.h>

#define MAP3D_WIDTH       (16u)
#define MAP3D_HEIGHT      (16u)
#define MAP3D_STORAGE_SIZE ((MAP3D_WIDTH * MAP3D_HEIGHT) / 8u)

typedef struct {
    uint8_t map[MAP3D_STORAGE_SIZE];
    uint32_t random_state;
    uint16_t player_x;
    uint16_t player_y;
    int16_t velocity_x;
    int16_t velocity_y;
    uint16_t simulation_accumulator_ms;
    uint16_t map_hold_ms;
    uint16_t map_blink_ms;
    int8_t angular_velocity;
    uint8_t angle;
    bool map_visible;
    bool map_hold_latched;
} map3d_game_t;

void map3d_game_init(void *context, uint32_t seed);
void map3d_game_deinit(void *context);
void map3d_game_update(void *context, uint8_t input_state,
                       uint32_t elapsed_ms);
void map3d_game_draw(const void *context);

#endif /* __MAP3D_GAME_H__ */
