/**
 * @file graphic_utils.h
 *
 * @brief Graphic utility functions
 *
 * This file contains general utils and wrappers that relate to
 * graphics/video/vram and generally displaying things on the screen.
 * Mostly wrappers and defines for using tonc.
 *
 * Note: the code here assumes we're working with a single screenblock
 * which should be true for this entire game since a screenblock
 * is enough to contain a full screen (and more)
 * and there isn't any scrolling etc.
 */
#ifndef GRAPHIC_UTILS_H
#define GRAPHIC_UTILS_H

#include <tonc_math.h>
#include <tonc_video.h>

/**
 * @name Graphics Utilities Constants
 *
 * Reminder:
 *  Screen Base Block is the base for the screenblock entries i.e. tilemap
 *  Character Base Block is the base for the tiles themselves
 *
 * @{
 */

/** @def MAIN_BG_SBB */
#define MAIN_BG_SBB 31

/** @def MAIN_BG_CBB */
#define MAIN_BG_CBB 1

/** @def TTE_SBB */
#define TTE_SBB 30

/** @def MAIN_BG_SBB */
#define TTE_CBB 0

/** @def AFFINE_BG_SBB */
#define AFFINE_BG_SBB 2

/** @def AFFINE_BG_CBB */
#define AFFINE_BG_CBB 2

/** @def PAL_ROW_LEN */
#define PAL_ROW_LEN 16

/** @def NUM_PALETTES */
#define NUM_PALETTES 16

/**
 * @def TILE_SIZE
 * @brief Tile size in pixels, both height and width as tiles are square
 */
#define TILE_SIZE 8

/**
 * @def EFFECT_TEXT_SEPARATION_AMOUNT
 *
 * @brief If we need to show multiple effects at once
 */
#define EFFECT_TEXT_SEPARATION_AMOUNT 32

/** @} */

/**
 * @name TTE Palette constants
 *
 * @{
 */

/** @def TTE_BIT_UNPACK_OFFSET */
#define TTE_BIT_UNPACK_OFFSET 14

/** @def TTE_BIT_ON_CLR_IDX */
#define TTE_BIT_ON_CLR_IDX TTE_BIT_UNPACK_OFFSET + 1

/** @def TTE_YELLOW_PB */
#define TTE_YELLOW_PB 12 // 0xC

/** @def TTE_BLUE_PB */
#define TTE_BLUE_PB 13 // 0xD

/** @def TTE_RED_PB */
#define TTE_RED_PB 14 // 0xE

/** @def TTE_WHITE_PB */
#define TTE_WHITE_PB 15 // 0xF

/** @def TTE_SPECIAL_PB_MULT_OFFSET */
#define TTE_SPECIAL_PB_MULT_OFFSET 0x1000

/**
 * @def TTE_CHAR_SIZE
 *
 * @brief By default TTE characters occupy a single tile
 */
#define TTE_CHAR_SIZE TILE_SIZE

/** @} */

/**
 * @name Text colors
 *
 * @{
 */

/** @def TEXT_CLR_YELLOW */
#define TEXT_CLR_YELLOW RGB15(31, 20, 0) // 0x029F

/** @def TEXT_CLR_BLUE */
#define TEXT_CLR_BLUE RGB15(0, 18, 31) // 0x7E40

/** @def TEXT_CLR_RED */
#define TEXT_CLR_RED RGB15(31, 9, 8) // 0x213F

/** @def TEXT_CLR_WHITE */
#define TEXT_CLR_WHITE CLR_WHITE

/** @} */

/**
 * @name Dimensions for a screenblock.
 *
 * A 1024 size screenblock is arranged in a grid of 32x32 screen entries
 * Interestingly since each block is 8x8 pixels, the 240x160 GBA screen
 * is smaller than the screenblock, only the top left part of the screenblock
 * is displayed on the screen.
 *
 * @{
 */

/** @def SE_ROW_LEN */
#define SE_ROW_LEN 32

/** @def SE_COL_LEN */
#define SE_COL_LEN 32

/** @} */

/**
 * @name Screen Directions
 *
 * Since y direction goes from the top of the screen to the bottom
 *
 * @{
 */

/** @def SCREEN_UP */
#define SCREEN_UP -1

/** @def SCREEN_DOWN */
#define SCREEN_DOWN 1

/** @def SCREEN_LEFT */
#define SCREEN_LEFT -1

/** @def SCREEN_RIGHT */
#define SCREEN_RIGHT 1

/**
 * @def SE_UP
 * @brief Screen Entry "up" direction
 */
#define SE_UP SCREEN_UP

/**
 * @def SE_DOWN
 * @brief Screen Entry "down" direction
 */
#define SE_DOWN SCREEN_DOWN

/** @def OVERFLOW_LEFT */
#define OVERFLOW_LEFT SCREEN_LEFT

/** @def OVERFLOW_RIGHT */
#define OVERFLOW_RIGHT SCREEN_RIGHT

/** @} */

// When making this, missed that it already exists in tonc_math.h
typedef RECT Rect;

/**
 * @brief Gets the screenblock entry for the given coordinates (x, y).
 *
 * @param pos a @ref BG_POINT with x and y in number of tiles.
 *
 * @return The screenblock entry.
 */
SE main_bg_se_get_se(BG_POINT pos);

/**
 * @brief Get the width of a rectangle
 *
 * @param rect a @ref Rect to measure
 *
 * @return The width of the rectangle.
 */
INLINE int rect_width(const Rect* rect)
{
    return (((rect)->right) - ((rect)->left) + 1);
}

/**
 * @brief Get the height of a rectangle
 *
 * @param rect a @ref Rect to measure
 *
 * @return The height of the rectangle.
 */
INLINE int rect_height(const Rect* rect)
{
    return (((rect)->bottom) - ((rect)->top) + 1);
}

/**
 * @brief Copies an SE rect vertically in direction by a single tile.
 *
 * NOTE: This does not work with TTE_SBB, probably because it's 4BPP...
 * 
 * If you are doing this operation you are probably doing this in the main
 * background and you should use main_bg_se_copy_rect_1_tile_vert() instead.
 *
 * @param bg_sbb the SBB of the background in which to move the rect: the
 * direction must be either SE_UP or SE_DOWN.
 *
 * @param se_rect dimensions are in number of tiles.
 *
 * @param direction must be either @ref SE_UP or @ref SE_DOWN.
 */
void bg_se_copy_rect_1_tile_vert(u16 bg_sbb, Rect se_rect, int direction);

/**
 * @brief Clears a rect in the main background.
 *
 * @param se_rect dimensions need to be in number of tiles.
 */
void main_bg_se_clear_rect(Rect se_rect);

/**
 * @brief Copies a rect in the main background vertically in direction by a single tile.
 *
 * @param se_rect dimensions are in number of tiles.
 *
 * @param direction must be either @ref SE_UP or @ref SE_DOWN.
 */
void main_bg_se_copy_rect_1_tile_vert(Rect se_rect, int direction);

/**
 * @brief Copies a rect in the main background from se_rect to the position (x, y).
 *
 * @param se_rect dimensions are in number of tiles.
 *
 * @param dest_pos x and y are the coordinates in number of tiles.
 */
void main_bg_se_copy_rect(Rect se_rect, BG_POINT dest_pos);

/**
 * @brief Copies a screen entry to a rect in the main background.
 *
 * The tile is copied to the top left corner of the rect.
 *
 * @param tile SE of the tile to copy from
 *
 * @param se_rect destination of copy, dimensions are in number of tiles.
 */
void main_bg_se_fill_rect_with_se(SE tile, Rect se_rect);

/**
 * @brief Copies a 3x3 rect and expands it to a passed size.
 *
 * Performs the following operation:
 *
 * 1. The 3x3 rect is stretched to fill se_rect_dest.
 * 2. The corners are copied
 * 3. The sides are stretched
 * 4. The center is filled.
 *
 * @param se_rect_dest destination for 3x3 copy, if rect sides are length 2, then the sides are not copied, only the
 * corners. **But dest rect sides must be at least 2.**
 *
 * @param se_rect_src_3x3_top_left points to the top left corner of the source 3x3 rect.
 */
void main_bg_se_copy_expand_3x3_rect(Rect se_rect_dest, BG_POINT se_rect_src_3x3_top_left);

/**
 * @brief Moves a rect in the main background vertically in direction by a single tile.
 *
 * Note that tiles in the previous location will be transparent (0x000)
 * so maybe copy would be a better choice if you don't want to delete things
 *
 * @param se_rect dimensions are in number of tiles.
 * @param direction must be either @ref SE_UP or @ref SE_DOWN.
 */
void main_bg_se_move_rect_1_tile_vert(Rect se_rect, int direction);

/**
 * @brief A wrapper for tte_erase_rect that would use the rect struct
 *
 * @param rect rectangle to erase
 */
void tte_erase_rect_wrapper(Rect rect);

/**
 * @brief Changes rect->left so it fits the digits of num exactly when right aligned to rect->right. Assumes num is not
 * negative.
 *
 * overflow_direction determines the direction the number will overflow
 * if it's too large to fit inside the rect.
 *
 * Note that both rect->left and rect-right need to be defined, top and bottom don't matter
 *
 * @param rect is in number of pixels but should be a multiple of TILE_SIZE, so it's a whole number of tiles to fit TTE
 * characters
 *
 * @param num number to display
 *
 * @param overflow_direction either OVERFLOW_LEFT or OVERFLOW_RIGHT.
 */
void update_text_rect_to_right_align_num(Rect* rect, int num, int overflow_direction);

/**
 * @brief Copies 16 bit data from src to dst, applying a palette offset to the data.
 *
 * This is intended solely for use with tile8/8bpp data for dst and src.
 * The palette offset allows the tiles to use a different location in the palette * memory
 * This is useful because  grit always loads the palette to the beginning of pal_bg_mem[]
 *
 * @param dst destination charblock
 *
 * @param src destination charblock
 *
 * @param wcount Number of words to copy
 *
 * @param palette_offset palette offset to shift to
 */
void memcpy16_tile8_with_palette_offset(u16* dst, const u16* src, uint hwcount, u8 palette_offset);

/**
 * @brief Copies 32 bit data from src to dst, applying a palette offset to the data.
 *
 * This is intended solely for use with tile8/8bpp data for dst and src.
 * The palette offset allows the tiles to use a different location in the palette memory
 * This is useful because grit always loads the palette to the beginning of pal_bg_mem[]
 *
 * @param dst destination charblock
 *
 * @param src destination charblock
 *
 * @param wcount Number of words to copy
 *
 * @param palette_offset palette offset to shift to
 */
void memcpy32_tile8_with_palette_offset(u32* dst, const u32* src, uint wcount, u8 palette_offset);

/**
 * @brief Toggles the visibility of the window layers.
 *
 * These windows are primarily used for the shadows on held jokers, consumables and cards.
 *
 * @param win0 the visibility state for window 0
 * @param win1 the visibility state for window 1
 */
void toggle_windows(bool win0, bool win1);

#endif // GRAPHIC_UTILS_H
