/**
 ******************************************************************************
 * @file    dino_game.h
 * @brief   Dino game module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __DINO_GAME_H__
#define __DINO_GAME_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define DINO_OBSTACLE_COUNT (2u)
#define DINO_CLOUD_COUNT    (3u)

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef enum {
    DINO_STATE_READY = 0,
    DINO_STATE_RUNNING,
    DINO_STATE_GAME_OVER,
} dino_state_t;

typedef enum {
    DINO_OBSTACLE_CACTUS_SMALL_BIG = 0,
    DINO_OBSTACLE_CACTUS_BIG_SMALL,
    DINO_OBSTACLE_CACTUS_BIG_BIG,
    DINO_OBSTACLE_CACTUS_THREE_SMALL,
    DINO_OBSTACLE_BIRD,
    DINO_OBSTACLE_TYPE_COUNT,
} dino_obstacle_type_t;

typedef struct {
    int16_t x_q4;
    uint8_t type;
    bool active;
} dino_obstacle_t;

typedef struct {
    int16_t x_q4;
    uint8_t y;
    uint8_t speed_divisor;
} dino_cloud_t;

typedef struct {
    dino_obstacle_t obstacles[DINO_OBSTACLE_COUNT];
    dino_cloud_t clouds[DINO_CLOUD_COUNT];
    uint32_t random_state;
    uint32_t score;
    uint32_t high_score;
    uint32_t score_accumulator_ms;
    uint32_t update_accumulator_ms;
    int16_t player_y_q4;
    int16_t player_velocity_q4;
    int16_t spawn_remaining_q4;
    uint16_t ground_phase_q4;
    uint8_t input_previous;
    uint8_t animation_frame;
    dino_state_t state;
    bool ducking;
    bool return_to_menu;
} dino_game_t;

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void dino_game_init(void *context, uint32_t seed);
void dino_game_deinit(void *context);
void dino_game_update(void *context, uint8_t input_state, uint32_t elapsed_ms);
void dino_game_draw(const void *context);

#ifdef __cplusplus
}
#endif

#endif /* __DINO_GAME_H__ */
