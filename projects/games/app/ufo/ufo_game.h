#ifndef __UFO_GAME_H__
#define __UFO_GAME_H__

#include <stdbool.h>
#include <stdint.h>

#define UFO_ENEMY_COUNT       (7u)
#define UFO_PLAYER_SHOT_COUNT (8u)
#define UFO_ENEMY_SHOT_COUNT  (8u)
#define UFO_STAR_COUNT        (18u)

typedef enum {
    UFO_STATE_READY = 0,
    UFO_STATE_PLAYING,
    UFO_STATE_GAME_OVER,
} ufo_state_t;

typedef struct {
    int16_t x;
    int8_t y;
    uint8_t type;
    uint8_t hp;
    uint8_t shot_timer;
    bool active;
} ufo_enemy_t;

typedef struct {
    int16_t x;
    int8_t y;
    bool active;
} ufo_projectile_t;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t speed;
} ufo_star_t;

typedef struct {
    ufo_enemy_t enemies[UFO_ENEMY_COUNT];
    ufo_projectile_t player_shots[UFO_PLAYER_SHOT_COUNT];
    ufo_projectile_t enemy_shots[UFO_ENEMY_SHOT_COUNT];
    ufo_star_t stars[UFO_STAR_COUNT];
    uint32_t random_state;
    uint32_t score;
    uint32_t high_score;
    uint16_t update_accumulator_ms;
    uint16_t spawn_timer_ms;
    uint16_t fire_timer_ms;
    uint16_t invincible_ms;
    uint16_t sound_timer_ms;
    uint8_t player_y;
    uint8_t hp;
    uint8_t bombs;
    uint8_t input_previous;
    bool sound_on;
    bool return_to_menu;
    ufo_state_t state;
} ufo_game_t;

void ufo_game_init(void *context, uint32_t seed);
void ufo_game_deinit(void *context);
void ufo_game_update(void *context, uint8_t input_state,
                     uint32_t elapsed_ms);
void ufo_game_draw(const void *context);

#endif /* __UFO_GAME_H__ */
