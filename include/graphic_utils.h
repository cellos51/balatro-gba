#ifndef GRAPHIC_UTILS_H
#define GRAPHIC_UTILS_H

#include <tonc_video.h>

/* This file contains general utils and wrappers that relate to 
 * graphics/video/vram and generally displaying things on the screen.
 * Mostly wrappers and defines for using tonc.
 * 
 * Note: the code here assumes we're working with a single screenblock 
 * which should be true for this entire game since a screenblock 
 * is enough to contain a full screen (and more)
 * and there isn't any scrolling etc.
 */

/* Reminder:
 *  Screen Base Block is the base for the screenblock entries i.e. tilemap
 *  Character Base Block is the base for the tiles themselves
 */
#define MAIN_BG_SBB 31 
#define MAIN_BG_CBB 1
#define TTE_SBB 30
#define TTE_CBB 0
#define AFFINE_BG_SBB 2
#define AFFINE_BG_CBB 2
#define PAL_ROW_LEN 16
#define NUM_PALETTES 16

#define TTE_BIT_UNPACK_OFFSET 14
#define TTE_BIT_ON_CLR_IDX TTE_BIT_UNPACK_OFFSET + 1

#define TTE_YELLOW_PB   12  // 0xC
#define TTE_BLUE_PB     13  // 0xD
#define TTE_RED_PB      14  // 0xE
#define TTE_WHITE_PB    15  // 0xF
#define TTE_SPECIAL_PB_MULT_OFFSET 0x1000 //TODO: Change to a better name?

#define TEXT_CLR_YELLOW RGB15(31, 20, 0)    // 0x029F
#define TEXT_CLR_BLUE   RGB15(0, 18, 31)    // 0x7E40
#define TEXT_CLR_RED    RGB15(31, 9, 8)     // 0x213F
#define TEXT_CLR_WHITE  CLR_WHITE

/* Dimensions for a screenblock.
 * A 1024 size screenblock is arranged in a grid of 32x32 screen entries
 * Interestingly since each block is 8x8 pixels, the 240x160 GBA screen
 * is smaller than the screenblock, only the top left part of the screenblock
 * is displayed on the screen.
 */
#define SE_ROW_LEN 32
#define SE_COL_LEN 32

// Since y direction goes from the top of the screen to the bottom
enum ScreenVertDir
{
    SCREEN_UP = -1,
    SCREEN_DOWN = 1
};

enum ScreenHorzDir
{
    SCREEN_LEFT = -1,
    SCREEN_RIGHT = 1
};

enum OverflowDir
{
    OVERFLOW_LEFT = SCREEN_LEFT,
    OVERFLOW_RIGHT = SCREEN_RIGHT
};

// Tile size in pixels, both height and width as tiles are square
#define TILE_SIZE 8
#define EFFECT_TEXT_SEPARATION_AMOUNT 32; // If we need to show multiple effects at once

// By default TTE characters occupy a single tile
#define TTE_CHAR_SIZE TILE_SIZE

// When making this, missed that it already exists in tonc_math.h
typedef RECT Rect;

/* Gets the screenblock entry for the given coordinates (x, y).
 * x and y are in number of tiles.
 * Returns the screenblock entry.
 */
SE main_bg_se_get_se(BG_POINT pos);

INLINE int rect_width(const Rect* rect)
{
    /* Extra parens to avoid issues in case compiler turns INLINE into macro
     * Not sure if necessary, could be just paranoia
     */ 
    return (((rect)->right) - ((rect)->left) + 1);
}

INLINE int rect_height(const Rect* rect)
{
    return (((rect)->bottom) - ((rect)->top) + 1);
}

/* Copies an SE rect vertically in direction by a single tile.
 * bg_sbb is the SBB of the background in which to move the rect
 * se_rect dimensions are in number of tiles.
 * 
 * NOTE: This does not work with TTE_SBB, probably because it's 4BPP...
 */
void bg_se_copy_rect_1_tile_vert(u16 bg_sbb, Rect se_rect, enum ScreenVertDir direction);

/* Clears a rect in the main background.
 * The se_rect dimensions need to be in number of tiles.
 */
void main_bg_se_clear_rect(Rect se_rect);

/* Copies a rect in the main background vertically in direction by a single tile.
 * se_rect dimensions are in number of tiles.
 */
void main_bg_se_copy_rect_1_tile_vert(Rect se_rect, enum ScreenVertDir direction);

/* Copies a rect in the main background from se_rect to the position (x, y).
 * se_rect dimensions are in number of tiles.
 * x and y are the coordinates in number of tiles.
 */
void main_bg_se_copy_rect(Rect se_rect, BG_POINT dest_pos);

/* Copies a screen entry to a rect in the main background.
 * se_rect dimensions are in number of tiles.
 * The tile is copied to the top left corner of the rect.
 */
void main_bg_se_fill_rect_with_se(SE tile, Rect se_rect);

/* Copies a 3x3 rect into se_rect_dest, the 3x3 rect is stretched to fill se_rect_dest. 
 * The corners are copied, the sides are stretched, and the center is filled.
 * The parameter se_rect_src_3x3_top_left points to the top left corner of the source
 * 3x3 rect.
 * Dest rect sides can be of length 2, then the sides are not copied, only the corners.
 * But dest rect sides must be at least 2.
 */
void main_bg_se_copy_expand_3x3_rect(Rect se_rect_dest, BG_POINT se_rect_src_3x3_top_left);

/* Moves a rect in the main background vertically in direction by a single tile.
 * Note that tiles in the previous location will be transparent (0x000)
 * so maybe copy would be a better choice if you don't want to delete things
 * se_rect dimensions are in number of tiles.
 */
void main_bg_se_move_rect_1_tile_vert(Rect se_rect, enum ScreenVertDir direction);

// A wrapper for tte_erase_rect that would use the rect struct
void tte_erase_rect_wrapper(Rect rect);

/* Changes rect->left so it fits a string exactly when right aligned to rect->right.
 * 
 * overflow_direction determines the direction the string will overflow
 * if it's too large to fit inside the rect.
 * 
 * The rect is in number of pixels but should be a multiple of TTE_CHAR_SIZE
 * so it's a whole number of tiles to fit TTE characters
 * 
 * Note that both rect->left and rect-right need to be defined, top and bottom don't matter
 */
void update_text_rect_to_right_align_str(Rect* rect, const char* str, enum OverflowDir overflow_direction);


/** 
 * @brief Updates a rect so a string is centered within it.
 * 
 * @param rect  The rect provided, the provided values are used to determine the center
 *              and it is then updated so the string starting in rect->left is centered
 *              The rect is in number of pixels but should be a multiple of TTE_CHAR_SIZE
 *              so it's a whole number of tiles to fit TTE characters.
 * 
 * @param str   The string, the center of the string will be at the center of the updated rect.
 * 
 * @param bias_direction    Which direction to bias when the string can't be evenly centered
 *                          with respect to char tiles.
 *                          Examples:
 *                          | |S|T|R| |     - Can be evenly centered, bias has no effect
 *                          | | |S|T|R| |   - Bias right
 *                          | |S|T|R| | |   - Bias left
 *                          |A|B|C|D| |     - Bias left
 *                          | |A|B|C|D|     - Bias right
 */
void update_text_rect_to_center_str(Rect* rect, const char* str, enum ScreenHorzDir bias_direction);

/*Copies 16 bit data from src to dst, applying a palette offset to the data.
 * This is intended solely for use with tile8/8bpp data for dst and src.
 * The palette offset allows the tiles to use a different location in the palette
 * memory
 * This is useful because  grit always loads the palette to the beginning of 
 * pal_bg_mem[]
 */
void memcpy16_tile8_with_palette_offset(u16* dst, const u16* src, uint hwcount, u8 palette_offset);

/*Copies 32 bit data from src to dst, applying a palette offset to the data.
 * This is intended solely for use with tile8/8bpp data for dst and src.
 * The palette offset allows the tiles to use a different location in the palette
 * memory
 * This is useful because  grit always loads the palette to the beginning of 
 * pal_bg_mem[]
 */
void memcpy32_tile8_with_palette_offset(u32* dst, const u32* src, uint wcount, u8 palette_offset);

/* Toggles the visibility of the window layers.
 * win0 and win1 are the visibility states for the two windows.
 * These windows are primarily used for the shadows on held jokers, consumables and cards.
 */
void toggle_windows(bool win0, bool win1);

#endif //GRAPHIC_UTILS_H
