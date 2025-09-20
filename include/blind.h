#ifndef BLIND_H
#define BLIND_H

#include "sprite.h"

#define MAX_ANTE 8 // The GBA's max uint value is around 4 billion, so we're going to not add endless mode for simplicity's sake

#define SMALL_BLIND_PB 1
#define BIG_BLIND_PB 2
#define BOSS_BLIND_PB 3

#define BLIND_SPRITE_OFFSET 16
#define SMALL_BLIND_TID 960
#define BIG_BLIND_TID (BLIND_SPRITE_OFFSET + SMALL_BLIND_TID)
#define BOSS_BLIND_TID (BLIND_SPRITE_OFFSET + BIG_BLIND_TID)

#define BLIND_TEXT_COLOR_INDEX 1
#define BLIND_SHADOW_COLOR_INDEX 2
#define BLIND_HIGHLIGHT_COLOR_INDEX 3
#define BLIND_MAIN_COLOR_INDEX 4
#define BLIND_BACKGROUND_MAIN_COLOR_INDEX 5
#define BLIND_BACKGROUND_SECONDARY_COLOR_INDEX 6
#define BLIND_BACKGROUND_SHADOW_COLOR_INDEX 7

enum BlindType
{
    SMALL_BLIND,
    BIG_BLIND,
    BOSS_BLIND,
    MAX_BLINDS
};

enum BlindState
{
    BLIND_CURRENT,
    BLIND_UPCOMING,
    BLIND_DEFEATED,
    BLIND_SKIPPED
};

// TODO: Move this to a common interface for other palettes
typedef struct {
    const u16* palette;
    u32 tid;
    u32 pb;
} PaletteInfo;

typedef struct
{
    enum BlindType type;
    PaletteInfo pal_info;
    FIXED multiplier;
    s32 reward;
} Blind;

void blind_init();

int blind_get_requirement(enum BlindType type, int ante);
int blind_get_reward(enum BlindType type);
u16 blind_get_color(enum BlindType type, int index);

Sprite *blind_token_new(enum BlindType type, int x, int y, int sprite_index);

#endif // BLIND_H
