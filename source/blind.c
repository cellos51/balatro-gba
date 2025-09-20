#include <tonc.h>

#include "blind.h"
#include "blinds_gfx.h"
#include "graphic_utils.h"

#define SMALL_BLIND_REWARD 3
#define BIG_BLIND_REWARD   4
#define BOSS_BLIND_REWARD  5

// +1 is added because we'll actually be indexing at 1, but if something causes you to go to ante 0, there will still be a value there.
static const int ante_lut[MAX_ANTE + 1] = {100, 300, 800, 2000, 5000, 11000, 20000, 35000, 50000};

// Palettes for the blinds (Transparency, Text Color, Shadow, Highlight, Main Color) Use this: http://www.budmelvin.com/dev/15bitconverter.html
static const u16 small_blind_token_palette[PAL_ROW_LEN] = {0x0000, 0x7FFF, 0x34A1, 0x5DCB, 0x5104, 0x55A0, 0x2D01, 0x34E0};
static const u16 big_blind_token_palette[PAL_ROW_LEN] = {0x0000, 0x2527, 0x15F5, 0x36FC, 0x1E9C, 0x01B4, 0x0D0A, 0x010E};
static const u16 boss_blind_token_palette[PAL_ROW_LEN] = {0x0000, 0x2CC9, 0x3D0D, 0x5E14, 0x5171, 0x4D0F, 0x2CC8, 0x3089}; // This variable is temporary, each boss blind will have its own unique palette

static const Blind blindMap[MAX_BLINDS] =
{
    {
        .type = SMALL_BLIND,
        .pal_info =
        {
            .palette = small_blind_token_palette,
            .tid = SMALL_BLIND_TID,
            .pb = SMALL_BLIND_PB,
        },
        .multiplier = FIX_ONE,
        .reward = SMALL_BLIND_REWARD,
    },
    {
        .type = BIG_BLIND,
        .pal_info =
        {
            .palette = big_blind_token_palette,
            .tid = BIG_BLIND_TID,
            .pb = BIG_BLIND_PB,
        },
        .multiplier = (FIX_ONE * 3) / 2,
        .reward = BIG_BLIND_REWARD,
    },
    {
        .type = BOSS_BLIND,
        .pal_info =
        {
            .palette = boss_blind_token_palette,
            .tid = BOSS_BLIND_TID,
            .pb = BOSS_BLIND_PB,
        },
        .multiplier = FIX_ONE * 2,
        .reward = BOSS_BLIND_REWARD,
    },
};

void blind_init()
{
    // Blind graphics (fighting grit every step of the way as usual)
    GRIT_CPY(&tile_mem[4][SMALL_BLIND_TID], blinds_gfxTiles);

    for(int i = 0; i < MAX_BLINDS; i++)
    {
        const u16* pal = blindMap[i].pal_info.palette;

        u32 pb = blindMap[i].pal_info.pb;
        u32 pal_offset = PAL_ROW_LEN * pb;

        memcpy16(&pal_obj_mem[pal_offset], pal, PAL_ROW_LEN);
    }
}

int blind_get_requirement(enum BlindType type, int ante)
{
    if (ante < 0 || ante > MAX_ANTE) ante = 0; // Ensure ante is within valid range

    return fx2int(blindMap[type].multiplier * ante_lut[ante]);
}

int blind_get_reward(enum BlindType type)
{
    return blindMap[type].reward;
}

u16 blind_get_color(enum BlindType type, int index)
{
    return blindMap[type].pal_info.palette[index];
}

Sprite *blind_token_new(enum BlindType type, int x, int y, int sprite_index)
{
    u16 a0 = ATTR0_SQUARE | ATTR0_4BPP;
    u16 a1 = ATTR1_SIZE_32x32;
    u32 tid = blindMap[type].pal_info.tid, pb = blindMap[type].pal_info.pb;

    Sprite* sprite = sprite_new(a0, a1, tid, pb, sprite_index);

    sprite_position(sprite, x, y);

    return sprite;
}
