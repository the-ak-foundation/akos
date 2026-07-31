#ifndef __KUNG_FU_GAME_H__
#define __KUNG_FU_GAME_H__

#include <stdbool.h>
#include <stdint.h>

#define KUNG_FU_ENEMY_COUNT (4u)
#define KUNG_FU_ARROW_COUNT (3u)

typedef enum {
    KUNG_FU_STATE_READY = 0,
    KUNG_FU_STATE_PLAYING,
    KUNG_FU_STATE_GAME_OVER,
    KUNG_FU_STATE_FINISHED,
} kung_fu_state_t;

typedef enum {
    KUNG_FU_FOE_FIST = 0,
    KUNG_FU_FOE_KNIFE,
    KUNG_FU_FOE_SWORD,
    KUNG_FU_FOE_SPEAR,
    KUNG_FU_FOE_ARCHER,
    KUNG_FU_FOE_DOG,
} kung_fu_foe_type_t;

typedef enum {
    KUNG_FU_ACTION_IDLE = 0,
    KUNG_FU_ACTION_RUN,
    KUNG_FU_ACTION_PUNCH,
    KUNG_FU_ACTION_KICK,
    KUNG_FU_ACTION_HIGH_KICK,
    KUNG_FU_ACTION_BLOCK,
    KUNG_FU_ACTION_HIT,
} kung_fu_action_t;

typedef struct {
    int16_t x;
    int16_t spawn_progress;
    uint16_t attack_timer_ms;
    uint8_t type;
    bool active;
    bool facing_right;
} kung_fu_foe_t;

typedef struct {
    int16_t x;
    int16_t y;
    int8_t vx;
    bool active;
} kung_fu_foe_t;

typedef struct {
    kung_fu_foe_t foes[KUNG_FU_ENEMY_COUNT];
    kung_fu_arrow_t arrows[KUNG_FU_ARROW_COUNT];
    uint32_t random_state;
    uint32_t score;
    uint32_t update_accumulator_ms;
    int16_t player_x;
    int16_t scroll_x;
    int16_t level_progress;
    uint16_t action_timer_ms;
    uint8_t action;
    uint8_t combo_step;
    uint8_t health;
    uint8_t level;
    uint8_t input_previous;
    uint8_t animation_frame;
    kung_fu_state_t state;
    bool facing_right;
    bool return_to_menu;
} kung_fu_game_t;

void kung_fu_game_init(void *context, uint32_t seed);
void kung_fu_game_deinit(void *context);
void kung_fu_game_update(void *context, uint8_t input_state,
                         uint32_t elapsed_ms);
void kung_fu_game_draw(const void *context);

#endif /* __KUNG_FU_GAME_H__ */
