#include "kung_fu_game.h"

#include <stddef.h>
#include <string.h>

#include "app.h"
#include "game_gfx.h"
#include "game_hal.h"
#include "kung_fu_bitmap.h"

#define KUNG_FU_UPDATE_MS          (33u)
#define KUNG_FU_GROUND_Y          (49)
#define KUNG_FU_PLAYER_MIN_X      (10)
#define KUNG_FU_PLAYER_MAX_X      (88)
#define KUNG_FU_PLAYER_Y          (KUNG_FU_GROUND_Y - 16)
#define KUNG_FU_ATTACK_MS         (180u)
#define KUNG_FU_LEVEL_LENGTH      (900)
#define KUNG_FU_MAX_LEVEL         (5u)
#define KUNG_FU_HEALTH_MAX        (100u)
#define KUNG_FU_HIT_DAMAGE        (2u)
#define KUNG_FU_DOG_HIT_HEIGHT    (8u)
#define KUNG_FU_FOE_SPEED_BASE    (1)
#define KUNG_FU_FOE_RIGHT_X       (136)
#define KUNG_FU_FOE_LEFT_X        (-16)
#define KUNG_FU_SPAWN_GAP_BASE    (96)
#define KUNG_FU_SPAWN_GAP_VAR     (96u)
#define KUNG_FU_TILE_SIZE         (16u)

static uint32_t kung_fu_random(kung_fu_game_t *game) {
    uint32_t value = game->random_state;

    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    game->random_state = (value == 0u) ? 0x4B554E47u : value;
    return game->random_state;
}

static uint8_t kung_fu_level_speed(const kung_fu_game_t *game) {
    return (uint8_t)(KUNG_FU_FOE_SPEED_BASE + ((game->level - 1u) / 2u));
}

static void kung_fu_reset_round(kung_fu_game_t *game,
                                kung_fu_state_t state) {
    memset(game->foes, 0, sizeof(game->foes));
    game->player_x = 24;
    game->scroll_x = 0;
    game->level_progress = 0;
    game->attack_timer_ms = 0u;
    game->health = KUNG_FU_HEALTH_MAX;
    game->input_previous = 0u;
    game->animation_frame = 0u;
    game->facing_right = true;
    game->return_to_menu = false;
    game->state = state;

    for (uint8_t index = 0u; index < KUNG_FU_ENEMY_COUNT; index++) {
        kung_fu_foe_t *foe = &game->foes[index];

        foe->spawn_progress = (int16_t)(
            50 + (int16_t)index *
            (KUNG_FU_SPAWN_GAP_BASE -
             (int16_t)((game->level > 1u) ? game->level * 8u : 0u)));
        foe->x = ((index & 1u) == 0u) ? KUNG_FU_FOE_RIGHT_X :
                 KUNG_FU_FOE_LEFT_X;
        foe->type = (uint8_t)(((index == 1u) && (game->level > 1u)) ?
                              KUNG_FU_FOE_DOG : KUNG_FU_FOE_STICKMAN);
        foe->active = false;
    }
}

static bool kung_fu_overlap(int16_t ax, int16_t ay, int16_t aw, int16_t ah,
                            int16_t bx, int16_t by, int16_t bw, int16_t bh) {
    return (ax < bx + bw) && (ax + aw > bx) &&
           (ay < by + bh) && (ay + ah > by);
}

static uint8_t kung_fu_foe_width(const kung_fu_foe_t *foe) {
    return (foe->type == KUNG_FU_FOE_DOG) ?
           KUNG_FU_BITMAP_WIDTH : KUNG_FU_BITMAP_WIDTH;
}

static uint8_t kung_fu_foe_height(const kung_fu_foe_t *foe) {
    return (foe->type == KUNG_FU_FOE_DOG) ?
           KUNG_FU_DOG_HIT_HEIGHT : KUNG_FU_BITMAP_HEIGHT;
}

static int16_t kung_fu_foe_y(const kung_fu_foe_t *foe) {
    return (foe->type == KUNG_FU_FOE_DOG) ?
           (KUNG_FU_GROUND_Y - KUNG_FU_DOG_HIT_HEIGHT) :
           (KUNG_FU_GROUND_Y - KUNG_FU_BITMAP_HEIGHT);
}

static void kung_fu_draw_bitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                                bool mirror) {
    for (uint8_t row = 0u; row < KUNG_FU_BITMAP_HEIGHT; row++) {
        int16_t screen_y = y + row;

        if ((screen_y < 0) || (screen_y >= (int16_t)GAME_DISPLAY_HEIGHT)) {
            continue;
        }
        for (uint8_t column = 0u; column < KUNG_FU_BITMAP_WIDTH; column++) {
            uint8_t source_column = mirror ?
                (uint8_t)(KUNG_FU_BITMAP_WIDTH - 1u - column) : column;
            uint8_t data = bitmap[(uint32_t)(row / 8u) *
                                  KUNG_FU_BITMAP_WIDTH + source_column];
            int16_t screen_x = x + column;

            if ((screen_x < 0) ||
                (screen_x >= (int16_t)GAME_DISPLAY_WIDTH)) {
                continue;
            }
            if ((data & (1u << (row & 7u))) != 0u) {
                game_display_draw_pixel((uint8_t)screen_x,
                                        (uint8_t)screen_y, true);
            }
        }
    }
}

static void kung_fu_respawn_foe(kung_fu_game_t *game, kung_fu_foe_t *foe) {
    bool right_side = (kung_fu_random(game) & 1u) != 0u;
    uint16_t level_gap = (uint16_t)(KUNG_FU_SPAWN_GAP_BASE -
                         ((game->level > 1u) ? game->level * 8u : 0u));

    if (level_gap < 40u) {
        level_gap = 40u;
    }
    foe->x = right_side ? KUNG_FU_FOE_RIGHT_X : KUNG_FU_FOE_LEFT_X;
    foe->type = ((kung_fu_random(game) % 4u) == 0u) ?
                KUNG_FU_FOE_DOG : KUNG_FU_FOE_STICKMAN;
    foe->spawn_progress = (int16_t)(
        game->level_progress + level_gap +
        (int16_t)(kung_fu_random(game) % KUNG_FU_SPAWN_GAP_VAR));
    foe->active = false;
}

static void kung_fu_update_foe(kung_fu_game_t *game, kung_fu_foe_t *foe) {
    uint8_t speed;
    int16_t foe_y;
    uint8_t foe_width;
    uint8_t foe_height;
    bool attacking;

    if (!foe->active) {
        if (game->level_progress >= foe->spawn_progress) {
            foe->active = true;
        }
        return;
    }

    speed = kung_fu_level_speed(game);
    if (foe->x < game->player_x) {
        foe->x += speed;
    } else if (foe->x > game->player_x) {
        foe->x -= speed;
    }

    foe_y = kung_fu_foe_y(foe);
    foe_width = kung_fu_foe_width(foe);
    foe_height = kung_fu_foe_height(foe);
    attacking = game->attack_timer_ms > 0u;

    if (attacking) {
        int16_t attack_x = game->facing_right ?
            (game->player_x + 12) : (game->player_x - 12);

        if (kung_fu_overlap(attack_x, KUNG_FU_PLAYER_Y + 5, 16, 8,
                            foe->x, foe_y, foe_width, foe_height)) {
            game->score += (foe->type == KUNG_FU_FOE_DOG) ? 15u : 10u;
            kung_fu_respawn_foe(game, foe);
            return;
        }
    }

    if (kung_fu_overlap(game->player_x + 4, KUNG_FU_PLAYER_Y + 1, 8, 15,
                        foe->x + 2, foe_y, foe_width - 3, foe_height)) {
        if (game->health > KUNG_FU_HIT_DAMAGE) {
            game->health = (uint8_t)(game->health - KUNG_FU_HIT_DAMAGE);
        } else {
            game->health = 0u;
            game->state = KUNG_FU_STATE_GAME_OVER;
        }
    }
}

static void kung_fu_draw_floor(void) {
    game_display_fill_rect(0u, KUNG_FU_GROUND_Y, GAME_DISPLAY_WIDTH,
                           2u, true);
    for (uint8_t x = 0u; x < GAME_DISPLAY_WIDTH; x += KUNG_FU_TILE_SIZE) {
        game_display_draw_pixel(x, (uint8_t)(KUNG_FU_GROUND_Y + 5), true);
        game_display_draw_pixel((uint8_t)(x + 6u),
                                (uint8_t)(KUNG_FU_GROUND_Y + 9), true);
        game_display_draw_pixel((uint8_t)(x + 11u),
                                (uint8_t)(KUNG_FU_GROUND_Y + 4), true);
    }
}

static void kung_fu_draw_hud(const kung_fu_game_t *game) {
    char level_text[4];

    level_text[0] = 'L';
    level_text[1] = (char)('0' + game->level);
    level_text[2] = '\0';
    game_gfx_draw_text(0u, 0u, "HP", 1u, true, false);
    game_display_fill_rect(14u, 1u, 52u, 5u, false);
    game_gfx_draw_rect(14u, 1u, 52u, 5u, true);
    game_display_fill_rect(15u, 2u, (uint8_t)(game->health / 2u), 3u, true);
    game_gfx_draw_text(84u, 0u, level_text, 1u, true, false);
    game_gfx_draw_number(104u, 0u, game->score, 1u, true, false);
}

static void kung_fu_draw_foes(const kung_fu_game_t *game) {
    for (uint8_t index = 0u; index < KUNG_FU_ENEMY_COUNT; index++) {
        const kung_fu_foe_t *foe = &game->foes[index];
        int16_t x = foe->x;

        if (!foe->active) {
            continue;
        }
        if ((x < 0) || (x >= (int16_t)GAME_DISPLAY_WIDTH)) {
            continue;
        }
        if (foe->type == KUNG_FU_FOE_DOG) {
            const uint8_t *bitmap = ((game->animation_frame >> 2) & 1u) ?
                kung_fu_dog_run_1 : kung_fu_dog_run_0;
            kung_fu_draw_bitmap(x, kung_fu_foe_y(foe) - 8, bitmap,
                                foe->x > game->player_x);
        } else {
            const uint8_t *bitmap = ((game->animation_frame >> 2) & 1u) ?
                kung_fu_enemy_run_1 : kung_fu_enemy_run_0;
            kung_fu_draw_bitmap(x, kung_fu_foe_y(foe), bitmap,
                                foe->x > game->player_x);
        }
    }
}

static void kung_fu_draw_player(const kung_fu_game_t *game) {
    const uint8_t *bitmap;

    if (game->attack_timer_ms > (KUNG_FU_ATTACK_MS * 2u / 3u)) {
        bitmap = kung_fu_player_kick_0;
    } else if (game->attack_timer_ms > (KUNG_FU_ATTACK_MS / 3u)) {
        bitmap = kung_fu_player_kick_1;
    } else if (game->attack_timer_ms > 0u) {
        bitmap = kung_fu_player_kick_2;
    } else {
        bitmap = ((game->animation_frame >> 2) & 1u) ?
            kung_fu_player_run_1 : kung_fu_player_run_0;
    }
    kung_fu_draw_bitmap(game->player_x, KUNG_FU_PLAYER_Y, bitmap,
                        !game->facing_right);
}

void kung_fu_game_init(void *context, uint32_t seed) {
    kung_fu_game_t *game = (kung_fu_game_t *)context;

    if (game == NULL) {
        return;
    }
    memset(game, 0, sizeof(*game));
    game->random_state = (seed == 0u) ? 0x4B554E47u : seed;
    game->level = 1u;
    kung_fu_reset_round(game, KUNG_FU_STATE_READY);
}

void kung_fu_game_deinit(void *context) {
    (void)context;
    game_buzzer_off();
}

void kung_fu_game_update(void *context, uint8_t input_state,
                         uint32_t elapsed_ms) {
    kung_fu_game_t *game = (kung_fu_game_t *)context;
    uint8_t pressed;

    if (game == NULL) {
        return;
    }

    pressed = (uint8_t)(input_state & ~game->input_previous);
    game->input_previous = input_state;

    if (game->state == KUNG_FU_STATE_READY) {
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            kung_fu_reset_round(game, KUNG_FU_STATE_PLAYING);
        }
        return;
    }

    if ((game->state == KUNG_FU_STATE_GAME_OVER) ||
        (game->state == KUNG_FU_STATE_FINISHED)) {
        game_buzzer_off();
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game->return_to_menu = true;
        }
        return;
    }

    game->update_accumulator_ms += elapsed_ms;
    while (game->update_accumulator_ms >= KUNG_FU_UPDATE_MS) {
        game->update_accumulator_ms -= KUNG_FU_UPDATE_MS;
        game->animation_frame++;

        if ((input_state & GAME_INPUT_UP) != 0u) {
            game->facing_right = false;
            if (game->player_x > KUNG_FU_PLAYER_MIN_X) {
                game->player_x -= 2;
            }
        }
        if ((input_state & GAME_INPUT_DOWN) != 0u) {
            game->facing_right = true;
            if (game->player_x < KUNG_FU_PLAYER_MAX_X) {
                game->player_x += 2;
            }
        }
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game->attack_timer_ms = KUNG_FU_ATTACK_MS;
        }
        if (game->attack_timer_ms > 0u) {
            if (game->attack_timer_ms > KUNG_FU_UPDATE_MS) {
                game->attack_timer_ms =
                    (uint16_t)(game->attack_timer_ms - KUNG_FU_UPDATE_MS);
            } else {
                game->attack_timer_ms = 0u;
            }
        }

        game->scroll_x = (int16_t)((game->scroll_x + 1) %
                                   KUNG_FU_TILE_SIZE);
        game->level_progress++;
        game->score++;

        for (uint8_t index = 0u; index < KUNG_FU_ENEMY_COUNT; index++) {
            kung_fu_update_foe(game, &game->foes[index]);
            if (game->state != KUNG_FU_STATE_PLAYING) {
                return;
            }
        }

        if (game->level_progress >= KUNG_FU_LEVEL_LENGTH) {
            if (game->level >= KUNG_FU_MAX_LEVEL) {
                game->state = KUNG_FU_STATE_FINISHED;
            } else {
                game->level++;
                kung_fu_reset_round(game, KUNG_FU_STATE_READY);
            }
            return;
        }
    }
}

void kung_fu_game_draw(const void *context) {
    const kung_fu_game_t *game = (const kung_fu_game_t *)context;

    if (game == NULL) {
        return;
    }
    game_display_clear();

    if (game->state == KUNG_FU_STATE_READY) {
        game_gfx_draw_text(21u, 12u, "KUNG FU", 2u, true, false);
        game_gfx_draw_text(27u, 34u, "ESCAPE", 2u, true, false);
        game_gfx_draw_text(21u, 55u, "MODE START", 1u, true, false);
        return;
    }

    kung_fu_draw_floor();
    kung_fu_draw_hud(game);
    kung_fu_draw_foes(game);
    kung_fu_draw_player(game);

    if (game->state == KUNG_FU_STATE_GAME_OVER) {
        game_display_fill_rect(22u, 20u, 84u, 24u, false);
        game_gfx_draw_rect(22u, 20u, 84u, 24u, true);
        game_gfx_draw_text(37u, 25u, "GAME OVER", 1u, true, false);
        game_gfx_draw_text(28u, 35u, "MODE MENU", 1u, true, false);
    } else if (game->state == KUNG_FU_STATE_FINISHED) {
        game_display_fill_rect(18u, 20u, 92u, 24u, false);
        game_gfx_draw_rect(18u, 20u, 92u, 24u, true);
        game_gfx_draw_text(34u, 25u, "ESCAPED!", 1u, true, false);
        game_gfx_draw_text(28u, 35u, "MODE MENU", 1u, true, false);
    }
}
