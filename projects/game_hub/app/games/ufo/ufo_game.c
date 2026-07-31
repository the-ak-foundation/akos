#include "ufo_game.h"

#include <stddef.h>
#include <string.h>

#include "app.h"
#include "game_gfx.h"
#include "game_hal.h"
#include "ufo_bitmap.h"

#define UFO_PLAYER_X      (5)
#define UFO_PLAYER_WIDTH  (16)
#define UFO_PLAYER_HEIGHT (16)
#define UFO_PLAYER_MIN_Y  (8u)
#define UFO_PLAYER_MAX_Y  (48u)
static uint32_t ufo_high_score;

static void ufo_sound(ufo_game_t* game, uint16_t frequency, uint16_t duration_ms) {
    game_buzzer_tone(frequency);
    game->sound_timer_ms = duration_ms;
    game->sound_on       = true;
}

static void ufo_update_sound(ufo_game_t* game, uint32_t elapsed_ms) {
    if (!game->sound_on) {
        return;
    }
    if (elapsed_ms >= game->sound_timer_ms) {
        game->sound_timer_ms = 0u;
        game->sound_on       = false;
        game_buzzer_off();
    }
    else {
        game->sound_timer_ms -= (uint16_t)elapsed_ms;
    }
}

static uint32_t ufo_random(ufo_game_t* game) {
    game->random_state = game->random_state * 1664525u + 1013904223u;
    return game->random_state;
}

static bool ufo_bitmap_pixel(const uint8_t* bitmap, uint8_t width, uint8_t height, int16_t x,
                             int16_t y) {
    if ((bitmap == NULL) || (x < 0) || (y < 0) || (x >= width) || (y >= height)) {
        return false;
    }
    return (bitmap[(uint16_t)(y / 8) * width + (uint16_t)x] & (1u << (y & 7))) != 0u;
}

static bool ufo_projectile_hits_bitmap(const ufo_projectile_t* shot, const uint8_t* bitmap,
                                       int16_t bitmap_x, int16_t bitmap_y, uint8_t width,
                                       uint8_t height) {
    for (uint8_t y = 0u; y < 2u; y++) {
        for (uint8_t x = 0u; x < 3u; x++) {
            if (ufo_bitmap_pixel(bitmap, width, height, (int16_t)(shot->x + x - bitmap_x),
                                 (int16_t)(shot->y + y - bitmap_y))) {
                return true;
            }
        }
    }
    return false;
}

static bool ufo_bitmaps_overlap(const uint8_t* first, int16_t first_x, int16_t first_y,
                                uint8_t first_width, uint8_t first_height, const uint8_t* second,
                                int16_t second_x, int16_t second_y, uint8_t second_width,
                                uint8_t second_height) {
    int16_t left   = (first_x > second_x) ? first_x : second_x;
    int16_t top    = (first_y > second_y) ? first_y : second_y;
    int16_t right  = (first_x + first_width < second_x + second_width) ? first_x + first_width
                                                                       : second_x + second_width;
    int16_t bottom = (first_y + first_height < second_y + second_height) ? first_y + first_height
                                                                         : second_y + second_height;

    for (int16_t y = top; y < bottom; y++) {
        for (int16_t x = left; x < right; x++) {
            if (ufo_bitmap_pixel(first, first_width, first_height, x - first_x, y - first_y) &&
                ufo_bitmap_pixel(second, second_width, second_height, x - second_x, y - second_y)) {
                return true;
            }
        }
    }
    return false;
}

static const uint8_t* ufo_enemy_bitmap(uint8_t type) {
    return (type == 0u) ? ufo_enemy_small : (type == 1u) ? ufo_enemy_medium : ufo_enemy_big;
}

static void ufo_reset(ufo_game_t* game, uint32_t seed) {
    memset(game, 0, sizeof(*game));
    game->random_state   = seed ^ 0x55464F21u;
    game->high_score     = ufo_high_score;
    game->player_y       = 31u;
    game->hp             = 3u;
    game->bombs          = 3u;
    game->spawn_timer_ms = 600u;
    game->state          = UFO_STATE_READY;
    for (uint8_t index = 0u; index < UFO_STAR_COUNT; index++) {
        game->stars[index].x     = (uint8_t)ufo_random(game);
        game->stars[index].y     = (uint8_t)(9u + ufo_random(game) % 55u);
        game->stars[index].speed = (uint8_t)(1u + ufo_random(game) % 3u);
    }
}

void ufo_game_init(void* context, uint32_t seed) {
    if (context != NULL) {
        ufo_reset((ufo_game_t*)context, seed);
    }
}

void ufo_game_deinit(void* context) {
    (void)context;
    game_buzzer_off();
}

static void ufo_spawn_enemy(ufo_game_t* game) {
    for (uint8_t index = 0u; index < UFO_ENEMY_COUNT; index++) {
        ufo_enemy_t* enemy = &game->enemies[index];
        if (!enemy->active) {
            uint8_t roll      = (uint8_t)(ufo_random(game) % 12u);
            enemy->type       = (game->score >= 50u && roll == 0u)  ? 3u
                                : (game->score >= 20u && roll < 3u) ? 2u
                                : (game->score >= 7u && roll < 7u)  ? 1u
                                                                    : 0u;
            enemy->x          = 127;
            enemy->y          = (int8_t)(10u + ufo_random(game) % (enemy->type == 3u ? 8u : 46u));
            enemy->hp         = (enemy->type == 3u)   ? 14u
                                : (enemy->type == 2u) ? 4u
                                : (enemy->type == 1u) ? 2u
                                                      : 1u;
            enemy->shot_timer = (uint8_t)(25u + ufo_random(game) % 40u);
            enemy->active     = true;
            return;
        }
    }
}

static void ufo_fire_player(ufo_game_t* game) {
    if (game->fire_timer_ms != 0u) {
        return;
    }
    for (uint8_t index = 0u; index < UFO_PLAYER_SHOT_COUNT; index++) {
        if (!game->player_shots[index].active) {
            game->player_shots[index].x = UFO_PLAYER_X + UFO_PLAYER_WIDTH;
            game->player_shots[index].y = (int8_t)(game->player_y + UFO_PLAYER_HEIGHT / 2u - 1u);
            game->player_shots[index].active = true;
            game->fire_timer_ms              = 180u;
            ufo_sound(game, 2200u, 45u);
            return;
        }
    }
}

static void ufo_fire_enemy(ufo_game_t* game, const ufo_enemy_t* enemy) {
    for (uint8_t index = 0u; index < UFO_ENEMY_SHOT_COUNT; index++) {
        if (!game->enemy_shots[index].active) {
            game->enemy_shots[index].x      = enemy->x;
            game->enemy_shots[index].y      = (int8_t)(enemy->y + 4);
            game->enemy_shots[index].active = true;
            return;
        }
    }
}

static void ufo_use_bomb(ufo_game_t* game) {
    if (game->bombs == 0u) {
        return;
    }
    game->bombs--;
    ufo_sound(game, 320u, 260u);
    for (uint8_t index = 0u; index < UFO_ENEMY_SHOT_COUNT; index++) {
        game->enemy_shots[index].active = false;
    }
    for (uint8_t index = 0u; index < UFO_ENEMY_COUNT; index++) {
        if (game->enemies[index].active) {
            if (game->enemies[index].type == 3u) {
                game->enemies[index].hp =
                    (game->enemies[index].hp > 3u) ? (uint8_t)(game->enemies[index].hp - 3u) : 0u;
            }
            else {
                game->enemies[index].hp = 0u;
            }
        }
    }
}

static void ufo_damage_player(ufo_game_t* game) {
    if (game->invincible_ms != 0u) {
        return;
    }
    if (game->hp > 0u) {
        game->hp--;
    }
    game->invincible_ms = 900u;
    ufo_sound(game, 240u, 180u);
    if (game->hp == 0u) {
        if (game->score > ufo_high_score) {
            ufo_high_score = game->score;
        }
        game->high_score = ufo_high_score;
        game->state      = UFO_STATE_GAME_OVER;
        ufo_sound(game, 140u, 420u);
    }
}

static void ufo_update_frame(ufo_game_t* game, uint8_t input_state) {
    if ((input_state & GAME_INPUT_UP) != 0u && game->player_y > UFO_PLAYER_MIN_Y) {
        game->player_y = (game->player_y >= UFO_PLAYER_MIN_Y + 2u) ? (uint8_t)(game->player_y - 2u)
                                                                   : UFO_PLAYER_MIN_Y;
    }
    if ((input_state & GAME_INPUT_DOWN) != 0u && game->player_y < UFO_PLAYER_MAX_Y) {
        game->player_y = (game->player_y <= UFO_PLAYER_MAX_Y - 2u) ? (uint8_t)(game->player_y + 2u)
                                                                   : UFO_PLAYER_MAX_Y;
    }
    if ((input_state & GAME_INPUT_MODE) != 0u) {
        ufo_fire_player(game);
    }

    for (uint8_t index = 0u; index < UFO_STAR_COUNT; index++) {
        ufo_star_t* star = &game->stars[index];
        if (star->x < star->speed) {
            star->x = 127u;
            star->y = (uint8_t)(9u + ufo_random(game) % 55u);
        }
        else {
            star->x = (uint8_t)(star->x - star->speed);
        }
    }

    for (uint8_t index = 0u; index < UFO_PLAYER_SHOT_COUNT; index++) {
        ufo_projectile_t* shot = &game->player_shots[index];
        if (shot->active) {
            shot->x += 4;
            if (shot->x > 127) {
                shot->active = false;
            }
        }
    }
    for (uint8_t index = 0u; index < UFO_ENEMY_SHOT_COUNT; index++) {
        ufo_projectile_t* shot = &game->enemy_shots[index];
        if (shot->active) {
            shot->x -= 2;
            if (shot->x < 0) {
                shot->active = false;
            }
            else if (ufo_projectile_hits_bitmap(shot, ufo_player, UFO_PLAYER_X, game->player_y,
                                                UFO_PLAYER_WIDTH, UFO_PLAYER_HEIGHT)) {
                shot->active = false;
                ufo_damage_player(game);
            }
        }
    }

    for (uint8_t enemy_index = 0u; enemy_index < UFO_ENEMY_COUNT; enemy_index++) {
        ufo_enemy_t* enemy = &game->enemies[enemy_index];
        const uint8_t* enemy_bitmap;
        uint8_t size;
        if (!enemy->active) {
            continue;
        }
        size         = (enemy->type >= 2u) ? 16u : 8u;
        enemy_bitmap = ufo_enemy_bitmap(enemy->type);
        enemy->x -= (enemy->type == 3u) ? 1 : 2;
        if (enemy->shot_timer > 0u) {
            enemy->shot_timer--;
        }
        else {
            ufo_fire_enemy(game, enemy);
            enemy->shot_timer = (enemy->type == 3u) ? 12u : 45u;
        }
        for (uint8_t shot_index = 0u; shot_index < UFO_PLAYER_SHOT_COUNT; shot_index++) {
            ufo_projectile_t* shot = &game->player_shots[shot_index];
            if (shot->active &&
                ufo_projectile_hits_bitmap(shot, enemy_bitmap, enemy->x, enemy->y, size, size)) {
                shot->active = false;
                if (enemy->hp > 0u) {
                    enemy->hp--;
                    ufo_sound(game, 1100u, 55u);
                }
            }
        }
        if (enemy->hp == 0u) {
            game->score += (enemy->type == 3u)   ? 100u
                           : (enemy->type == 2u) ? 10u
                           : (enemy->type == 1u) ? 5u
                                                 : 1u;
            enemy->active = false;
            ufo_sound(game, 1650u, 90u);
        }
        else if (enemy->x + size < 0) {
            enemy->active = false;
        }
        else if (ufo_bitmaps_overlap(enemy_bitmap, enemy->x, enemy->y, size, size, ufo_player,
                                     UFO_PLAYER_X, game->player_y, UFO_PLAYER_WIDTH,
                                     UFO_PLAYER_HEIGHT)) {
            enemy->active = false;
            ufo_damage_player(game);
        }
    }
}

void ufo_game_update(void* context, uint8_t input_state, uint32_t elapsed_ms) {
    ufo_game_t* game = (ufo_game_t*)context;
    uint8_t pressed;

    if (game == NULL) {
        return;
    }
    ufo_update_sound(game, elapsed_ms);
    pressed              = (uint8_t)(input_state & ~game->input_previous);
    game->input_previous = input_state;
    if (game->state == UFO_STATE_READY) {
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game->state = UFO_STATE_PLAYING;
            ufo_sound(game, 900u, 120u);
        }
        return;
    }
    if (game->state == UFO_STATE_GAME_OVER) {
        if ((pressed & GAME_INPUT_MODE) != 0u) {
            game->return_to_menu = true;
        }
        else if ((pressed & (GAME_INPUT_UP | GAME_INPUT_DOWN)) != 0u) {
            ufo_reset(game, game->score + game->random_state);
        }
        return;
    }

    if ((input_state & (GAME_INPUT_UP | GAME_INPUT_DOWN)) == (GAME_INPUT_UP | GAME_INPUT_DOWN)) {
        if ((pressed & (GAME_INPUT_UP | GAME_INPUT_DOWN)) != 0u) {
            ufo_use_bomb(game);
        }
    }
    game->fire_timer_ms =
        (elapsed_ms >= game->fire_timer_ms) ? 0u : (uint16_t)(game->fire_timer_ms - elapsed_ms);
    game->invincible_ms =
        (elapsed_ms >= game->invincible_ms) ? 0u : (uint16_t)(game->invincible_ms - elapsed_ms);
    if (elapsed_ms >= game->spawn_timer_ms) {
        ufo_spawn_enemy(game);
        game->spawn_timer_ms = (game->score < 30u) ? 700u : 480u;
    }
    else {
        game->spawn_timer_ms -= (uint16_t)elapsed_ms;
    }
    game->update_accumulator_ms += (uint16_t)elapsed_ms;
    while (game->update_accumulator_ms >= 33u) {
        game->update_accumulator_ms -= 33u;
        ufo_update_frame(game, input_state);
        if (game->state != UFO_STATE_PLAYING) {
            break;
        }
    }
}

static void ufo_draw_projectile(const ufo_projectile_t* shot) {
    if (shot->active && shot->x >= 0 && shot->x < 128 && shot->y >= 8 && shot->y < 64) {
        game_display_fill_rect((uint8_t)shot->x, (uint8_t)shot->y, 3u, 2u, true);
    }
}

void ufo_game_draw(const void* context) {
    const ufo_game_t* game = (const ufo_game_t*)context;

    if (game == NULL) {
        return;
    }
    game_display_clear();
    if (game->state == UFO_STATE_READY) {
        game_gfx_draw_text(46u, 20u, "UFO", 2u, true, false);
        game_gfx_draw_text(32u, 53u, "MODE:START", 1u, true, false);
        return;
    }
    if (game->state == UFO_STATE_GAME_OVER) {
        game_gfx_draw_text(10u, 13u, "GAME OVER", 2u, true, false);
        game_gfx_draw_text(25u, 36u, "SCORE", 1u, true, false);
        game_gfx_draw_number(62u, 36u, game->score, 1u, true, false);
        game_gfx_draw_text(5u, 54u, "UP:RETRY MODE:MENU", 1u, true, false);
        return;
    }

    for (uint8_t index = 0u; index < UFO_STAR_COUNT; index++) {
        game_display_draw_pixel(game->stars[index].x, game->stars[index].y, true);
    }
    if ((game->invincible_ms == 0u) || ((game->invincible_ms / 100u) & 1u)) {
        game_display_draw_bitmap(UFO_PLAYER_X, game->player_y, ufo_player, UFO_PLAYER_WIDTH,
                                 UFO_PLAYER_HEIGHT, true);
    }
    for (uint8_t index = 0u; index < UFO_ENEMY_COUNT; index++) {
        const ufo_enemy_t* enemy = &game->enemies[index];
        if (enemy->active && enemy->x >= 0 && enemy->x < 128) {
            const uint8_t* bitmap = ufo_enemy_bitmap(enemy->type);
            uint8_t size          = (enemy->type >= 2u) ? 16u : 8u;
            game_display_draw_bitmap((uint8_t)enemy->x, (uint8_t)enemy->y, bitmap, size, size,
                                     true);
        }
    }
    for (uint8_t index = 0u; index < UFO_PLAYER_SHOT_COUNT; index++) {
        ufo_draw_projectile(&game->player_shots[index]);
    }
    for (uint8_t index = 0u; index < UFO_ENEMY_SHOT_COUNT; index++) {
        ufo_draw_projectile(&game->enemy_shots[index]);
    }

    game_display_fill_rect(0u, 0u, 128u, 8u, false);
    game_display_draw_bitmap(0u, 0u, ufo_heart, 8u, 8u, true);
    game_gfx_draw_number(9u, 0u, game->hp, 1u, true, false);
    game_display_draw_bitmap(25u, 0u, ufo_bomb, 8u, 8u, true);
    game_gfx_draw_number(34u, 0u, game->bombs, 1u, true, false);
    game_gfx_draw_text(55u, 0u, "S:", 1u, true, false);
    game_gfx_draw_number(68u, 0u, game->score, 1u, true, false);
}
