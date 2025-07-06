#include <tonc.h>
#include <maxmod.h>
#include <string.h>

#include "sprite.h"
#include "card.h"
#include "game.h"
#include "blind.h"

// Graphics
#include "deck_gfx.h"
#include "background_gfx.h"
#include "background_play_gfx.h"
#include "affine_background_gfx.h"

// Audio
#include "soundbank.h"
#include "soundbank_bin.h"

#define MAP_AFF_SIZE 0x0100
BG_AFFINE bgaff;
AFF_SRC_EX asx = {32<<8, 64<<8, 120, 80, 0x0100, 0x0100, 0};


void init_map() // Temp function ripped from libtonc example
{   
    // Grit is fucking garbage and doesn't allow you to have a palette offset, they all get loaded at 0x0000. I have no manually change each tile's palette bank

    unsigned int correctedTiles[affine_background_gfxTilesLen / 4];
    for (int i = 0; i < affine_background_gfxTilesLen / 4; i++)
    {   
        correctedTiles[i] = affine_background_gfxTiles[i] | 0xF0F0F0F0; // I brute forced this. I have no idea how this would and does work, but it does. We'll just leave this here for now.
        // Actually for future reference, each number seems to equate to an XY on the palette, so 0xF0F0F0F0 would just push everything down 15 colors, since the second number isn't set.
    }

	memcpy(&tile8_mem[2], correctedTiles, affine_background_gfxTilesLen);
    GRIT_CPY(&se_mem[2], affine_background_gfxMap);
    memcpy16(&pal_bg_mem[16 * 15], affine_background_gfxPal, 5);

	bgaff = bg_aff_default;
}

void init()
{
    irq_init(NULL);
    irq_add(II_VBLANK, mmVBlank);

    // Initialize maxmod
    mmInitDefault((mm_addr)soundbank_bin, 12);
    mmStart(MOD_MAIN_THEME, MM_PLAY_LOOP);

    // Initialize text engine
    tte_init_se(0, BG_CBB(0) | BG_SBB(30), 0, CLR_WHITE, 14, NULL, NULL);
    tte_erase_screen();
    tte_init_con();

    // TTE palette setup
    pal_bg_bank[12][15] = 0x029F; // Yellow. honestly fuck libtonc because i cannot figure out how you're supposed to select a color from the palette index so i'm doing it like this
    pal_bg_bank[13][15] = 0x7E40; // Blue
    pal_bg_bank[14][15] = 0x213F; // Red
    pal_bg_bank[15][15] = CLR_WHITE;

    // Load the tiles and palette
    // Background
    memcpy(pal_bg_mem, background_gfxPal, 64); // This '64" isn't a specific number, I'm just using it to prevent the text colors from being overridden
    GRIT_CPY(&tile8_mem[1], background_gfxTiles); // Deadass i have no clue how any of these memory things work but I just messed with them until stuff worked
    GRIT_CPY(&se_mem[31], background_gfxMap);

    // Deck graphics
    GRIT_CPY(&tile_mem[4], deck_gfxTiles);
    GRIT_CPY(pal_obj_mem, deck_gfxPal);

    // Set up the video mode
    REG_BG0CNT = BG_PRIO(0) | BG_CBB(0) | BG_SBB(30) | BG_4BPP;
    REG_BG1CNT = BG_PRIO(1) | BG_CBB(1) | BG_SBB(31) | BG_8BPP;
    REG_BG2CNT = BG_PRIO(2) | BG_CBB(2) | BG_SBB(2) | BG_8BPP | BG_WRAP | BG_AFF_32x32;

    int win1_left = 72;
    int win1_top = 44;
    int win1_right = 200;
    int win1_bottom = 160;

    int win2_left = 72;
    int win2_top = 0;
    int win2_right = 232;
    int win2_bottom = 44;

	REG_WIN0H = win1_left<<8 | win1_right;
	REG_WIN0V =  win1_top<<8 | win1_bottom;
	REG_WIN0CNT = WIN_ALL | WIN_BLD;
	REG_WINOUTCNT = WIN_ALL;

    REG_WIN1H = win2_left<<8 | win2_right;
    REG_WIN1V =  win2_top<<8 | win2_bottom;
    REG_WIN1CNT = WIN_ALL | WIN_BLD;

    REG_BLDCNT = BLD_BUILD(BLD_BG1, BLD_BG2, 1);

    REG_BLDALPHA = BLDA_BUILD(0, 13);

    REG_DISPCNT = DCNT_MODE1 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_OBJ | DCNT_WIN0 | DCNT_WIN1;

    init_map(); // Initialize the map for the background

    // Initialize subsystems
    sprite_init();
    blinds_init();
    game_init();
}

void background_update() // This needs to be called before update() and draw() because setting REG_BG_AFFINE during an HBlank can cause issues
{
    static uint timer = 0;
    timer++;

    // These values are not permament, just the current configuration for the affine background
    asx.tex_x += 5;
    asx.tex_y += 12;
    asx.sx = (lu_sin(timer * 100)) >> 8; // Scale the sine value to fit in a s16
    asx.sx += 256; // Add 256 to the sine value to make it positive
    asx.sy = (lu_sin(timer * 100 + 0x4000)) >> 8; // Scale the sine value to fit in a s16
    asx.sy += 256; // Add 256 to the sine value to make it positive
    bg_rotscale_ex(&bgaff, &asx);
	REG_BG_AFFINE[2]= bgaff;
}

void update()
{
    game_update();
}

void draw()
{
    sprite_draw();
}

int main()
{
    init();

	while(true)
    {
        VBlankIntrWait();
        mmFrame();
		key_poll();
        background_update();
        update();
        draw();
    }

	return 0;
}