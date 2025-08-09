#include "affine_background.h"
#include "affine_background_gfx.h"

#include "graphic_utils.h"

BG_AFFINE bgaff;
AFF_SRC_EX asx = {32<<8, 64<<8, 120, 80, 0x0100, 0x0100, 0};
static uint timer = 0;

#define TOP_SCANLINE_OFFSET 228
#define ANIMATION_SPEED_DIVISOR 16


void affine_background_init()
{
    memcpy32_tile8_with_palette_offset((u32*)&tile8_mem[AFFINE_BG_CBB], (const u32*)affine_background_gfxTiles, affine_background_gfxTilesLen/4, AFFINE_BG_PB);
    GRIT_CPY(&se_mem[AFFINE_BG_SBB], affine_background_gfxMap);
    affine_background_load_palette(affine_background_gfxPal);

    bgaff = bg_aff_default;
}

void affine_background_hblank()
{
    vu16 vcount = REG_VCOUNT;

    // This fixes a visual issue where the top 3 pixels of the scanline are out of sync
    if (vcount >= 160)
    {
        vcount -= TOP_SCANLINE_OFFSET;
    }

    const s32 timer_s32 = timer << 8;
    const s32 vcount_s32 = vcount << 8;
    const s16 vcount_s16 = vcount;
    const s32 vcount_sine = lu_sin(vcount_s32 + timer_s32 / ANIMATION_SPEED_DIVISOR); // dividing the timer by 16 to make the animation slower

    asx.scr_x = (SCREEN_WIDTH / 2); // 128 on x and y is an offset used to center the rotation
    asx.scr_y = vcount_s16 - (SCREEN_HEIGHT / 2); // scr_y must equal vcount otherwise the background will have no vertical difference
    asx.tex_x = vcount_sine;
    asx.tex_y = vcount_sine / 64;
    asx.sx = vcount_sine / 32;
    asx.sy = vcount_sine / 16;
    asx.alpha = vcount_sine + (timer_s32 / ANIMATION_SPEED_DIVISOR);
    
    bg_rotscale_ex(&bgaff, &asx);
    REG_BG_AFFINE[2] = bgaff;
}

void affine_background_update()
{
    timer++;
}

void affine_background_set_color(COLOR color)
{
    memcpy16(&pal_bg_mem[AFFINE_BG_PB], affine_background_gfxPal, AFFINE_BG_PAL_LEN);
    for (int i = 0; i < AFFINE_BG_PAL_LEN; i++)
    {
        clr_rgbscale(&pal_bg_mem[AFFINE_BG_PB] + i, &pal_bg_mem[AFFINE_BG_PB] + i, 1, color);
    }
}

void affine_background_load_palette(const u16 *src)
{
    memcpy16(&pal_bg_mem[AFFINE_BG_PB], src, AFFINE_BG_PAL_LEN);
}