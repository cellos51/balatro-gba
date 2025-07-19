#ifndef GRAPHIC_UTILS_H
#define GRAPHIC_UTILS_H

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
#define MAIN_BG_SBB 29 
#define MAIN_BG_CBB 1
#define TTE_SBB 30
#define TTE_CBB 0
#define AFFINE_BG_SBB 2
#define AFFINE_BG_CBB 2
#define PAL_ROW_LEN 16

/* Dimensions for a screenblock.
 * A 1024 size screenblock is arranged in a grid of 32x32 screen entries
 * Interestingly since each block is 8x8 pixels, the 240x160 GBA screen
 * is smaller than the screenblock, only the top left part of the screenblock
 * is displayed on the screen.
 */
#define SE_ROW_LEN 32
#define SE_COL_LEN 32

// Since y direction goes from the top of the screen to the bottom
#define SE_UP	-1
#define SE_DOWN 1

#define OVERFLOW_LEFT	-1
#define OVERFLOW_RIGHT	1

// Tile size in pixels, both height and width as tiles are square
#define TILE_SIZE 8

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} Rect;

#define UNDEFINED -1

/* Gets the screenblock tile for the given coordinates (x, y).
 * x and y are in number of tiles.
 * Returns the screenblock tile.
 */
u16 main_bg_se_get_tile(BG_POINT pos);

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

/* Clears a rect in the main background.
 * The se_rect dimensions need to be in number of tiles.
 */
void main_bg_se_clear_rect(Rect se_rect);

/* Copies a rect in the main background vertically in direction by a single tile.
 * direction must be either SE_UP or SE_DOWN.
 * se_rect dimensions are in number of tiles.
 */
void main_bg_se_copy_rect_1_tile_vert(Rect se_rect, int direction);

/* Copies a rect in the main background from se_rect to the position (x, y).
 * se_rect dimensions are in number of tiles.
 * x and y are the coordinates in number of tiles.
 */
void main_bg_se_copy_rect(Rect se_rect, BG_POINT pos);

/* Copies a tile to a rect in the main background.
 * se_rect dimensions are in number of tiles.
 * The tile is copied to the top left corner of the rect.
 */
void main_bg_se_copy_tile_to_rect(u16 tile, Rect se_rect);
/* Moves a rect in the main background vertically in direction by a single tile.
 * Note that tiles in the previous location will be transparent (0x000)
 * so maybe copy would be a better choice if you don't want to delete things
 * direction must be either SE_UP or SE_DOWN.
 * se_rect dimensions are in number of tiles.
 */
void main_bg_se_move_rect_1_tile_vert(Rect se_rect, int direction);

// A wrapper for tte_erase_rect that would use the rect struct
void tte_erase_rect_wrapper(Rect rect);

/* Changes rect->left so it fits the digits of num exactly when right aligned to rect->right.
 * Assumes num is not negative.
 * 
 * overflow_direction determines the direction the number will overflow
 * if it's too large to fit inside the rect. 
 * Should be either OVERFLOW_LEFT or OVERFLOW_RIGHT.
 * 
 * The rect is in number of pixels but should be a multiple of TILE_SIZE
 * so it's a whole number of tiles to fit TTE characters
 * 
 * Note that both rect->left and rect-right need to be defined, top and bottom don't matter
 */
void update_text_rect_to_right_align_num(Rect* rect, int num, int overflow_direction);

#endif //GRAPHIC_UTILS_H