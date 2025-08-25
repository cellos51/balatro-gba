#include "affine_background.h"
#include "affine_background_gfx.h"

#include "graphic_utils.h"

BG_AFFINE bgaff_arr[SCREEN_HEIGHT + 1];

static uint timer = 0;

#define ANIMATION_SPEED_DIVISOR 16


void affine_background_init()
{
    memcpy32_tile8_with_palette_offset((u32*)&tile8_mem[AFFINE_BG_CBB], (const u32*)affine_background_gfxTiles, affine_background_gfxTilesLen/4, AFFINE_BG_PB);
    GRIT_CPY(&se_mem[AFFINE_BG_SBB], affine_background_gfxMap);
    affine_background_load_palette(affine_background_gfxPal);

    REG_BG_AFFINE[AFFINE_BG_IDX] = bg_aff_default;
}

// Pre-computes the affine matrices values for each scanline 
// and stores in bgaff_arr. 
// This is to be done in VBLANK so the HBLANK code 
// can just fetch the values quickly.
IWRAM_CODE void affine_background_prep_bgaff_arr()
{
    for (u16 vcount = 0; vcount < SCREEN_HEIGHT; vcount++)
    {
        AFF_SRC_EX asx = {32<<8, 64<<8, 120, 80, 0x0100, 0x0100, 0};
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

        bg_rotscale_ex(&bgaff_arr[vcount], &asx);
    }

    /* HBLANK occurs after the scanline so REG_VCOUNT represents the 
     * the scanline that just passed, so when it's SCREEN_HEIGHT we will
     * actually be updating the first line
     */
    bgaff_arr[SCREEN_HEIGHT] = bgaff_arr[0];
}

IWRAM_CODE void affine_background_hblank()
{
    vu16 vcount = REG_VCOUNT;

    if ((vcount >= SCREEN_HEIGHT)) // Exit the function if the scanline is outside the screen
    {
        return;
    }

    // See comment in affine_background_prep_bgaff_arr()
    REG_BG_AFFINE[AFFINE_BG_IDX] = bgaff_arr[vcount + 1];
}

void affine_background_update()
{
    affine_background_prep_bgaff_arr();
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