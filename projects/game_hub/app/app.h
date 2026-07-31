#ifndef __GAME_APP_H__
#define __GAME_APP_H__

#include <stdint.h>

#include "akos.h"

#define GAME_FRAME_PERIOD_TICKS (16u)
#define GAME_DISPLAY_PERIOD_TICKS (33u)
#define GAME_TASK_STACK_SIZE    (256u)
#define GAME_LIFE_STACK_SIZE    (64u)
#define GAME_SHELL_STACK_SIZE   (256u)

typedef enum {
    GAME_THREAD_LOGIC = 0,
    GAME_THREAD_LIFE,
    GAME_THREAD_SHELL,
    GAME_THREAD_COUNT
} game_thread_id_t;

enum {
    GAME_INPUT_UP   = 1u << 0,
    GAME_INPUT_DOWN = 1u << 1,
    GAME_INPUT_MODE = 1u << 2,
};

void task_game_hub(void *argument);
void task_life(void *argument);
void game_app_display_pause(void);
void game_app_display_resume(void);

#endif /* __GAME_APP_H__ */
