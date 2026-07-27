#ifndef __GAME_HUB_LOGO_H__
#define __GAME_HUB_LOGO_H__

#include <stdint.h>

#define GAME_HUB_LOGO_WIDTH  (57u)
#define GAME_HUB_LOGO_HEIGHT (64u)
#define GAME_HUB_LOGO_SIZE \
    (GAME_HUB_LOGO_WIDTH * ((GAME_HUB_LOGO_HEIGHT + 7u) / 8u))

extern const uint8_t game_hub_logo[GAME_HUB_LOGO_SIZE];

#endif /* __GAME_HUB_LOGO_H__ */
