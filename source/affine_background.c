#include "affine_background.h"
#include "affine_background_gfx.h"
#include "affine_main_menu_background_gfx.h"

#include "graphic_utils.h"

BG_AFFINE bgaff;
AFF_SRC_EX asx = {0};

enum AffineBackgroundID background = AFFINE_BG_MAIN_MENU;

void affine_background_init()
{   
    affine_background_change_background(background);
    affine_background_update();

    bgaff = bg_aff_default;
}

void affine_background_update()
{
    static uint timer = 0;
    timer++;

    switch (background)
    {
    case AFFINE_BG_MAIN_MENU:
        asx.scr_x = SCREEN_WIDTH / 2;
        asx.scr_y = SCREEN_HEIGHT / 2;
        asx.tex_x = 16560;
        asx.tex_y = 14400;
        asx.sx = 128;
        asx.sy = 128;
        asx.alpha = 1000; // This slightly rotates it which increases the perceived resolution of the background
        break;
    case AFFINE_BG_GAME:
        asx.scr_x = 0;
        asx.scr_y = 0;
        asx.tex_x += 5;
        asx.tex_y += 12;
        asx.sx = (lu_sin(timer * 100)) >> 8; // Scale the sine value to fit in a s16
        asx.sx += 256; // Add 256 to the sine value to make it positive
        asx.sy = (lu_sin(timer * 100 + 0x4000)) >> 8; // Scale the sine value to fit in a s16
        asx.sy += 256; // Add 256 to the sine value to make it positive
        asx.alpha = 0;
        break;
    }

    bg_rotscale_ex(&bgaff, &asx);
    REG_BG_AFFINE[2] = bgaff;
}

void affine_background_set_color(COLOR color)
{
    affine_background_change_background(background); // Reload the palette to reset any previous color scaling
    for (int i = 0; i < AFFINE_BG_PAL_LEN; i++)
    {
        clr_rgbscale(&pal_bg_mem[AFFINE_BG_PB] + i, &pal_bg_mem[AFFINE_BG_PB] + i, 1, color);
    }
}

void affine_background_load_palette(const u16 *src)
{
    memcpy16(&pal_bg_mem[AFFINE_BG_PB], src, AFFINE_BG_PAL_LEN);
}

void affine_background_change_background(enum AffineBackgroundID new_bg)
{
    background = new_bg;

    switch (background)
    {
    case AFFINE_BG_MAIN_MENU:
        REG_BG2CNT &= ~BG_AFF_32x32;
        REG_BG2CNT |= BG_AFF_16x16;

        memcpy32_tile8_with_palette_offset((u32*)&tile8_mem[AFFINE_BG_CBB], (const u32*)affine_main_menu_background_gfxTiles, affine_main_menu_background_gfxTilesLen/4, AFFINE_BG_PB);
        GRIT_CPY(&se_mem[AFFINE_BG_SBB], affine_main_menu_background_gfxMap);
        affine_background_load_palette(affine_main_menu_background_gfxPal);
        break;
    case AFFINE_BG_GAME:
        REG_BG2CNT &= ~BG_AFF_16x16;
        REG_BG2CNT |= BG_AFF_32x32;

        memcpy32_tile8_with_palette_offset((u32*)&tile8_mem[AFFINE_BG_CBB], (const u32*)affine_background_gfxTiles, affine_background_gfxTilesLen/4, AFFINE_BG_PB);
        GRIT_CPY(&se_mem[AFFINE_BG_SBB], affine_background_gfxMap);
        affine_background_load_palette(affine_background_gfxPal);
        break;
    }
}