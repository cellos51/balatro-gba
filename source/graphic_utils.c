#include <tonc_core.h>
#include <tonc_tte.h>
#include <tonc_math.h>

#include "graphic_utils.h"

const Rect FULL_SCREENBLOCK_RECT = { 0, 0, SE_ROW_LEN, SE_COL_LEN };

// Clips a rect of screenblock entries to a specified rect
// The bounding rect is not required to be within screenblock boundaries
static void clip_se_rect_to_bounding_rect(Rect* rect, const Rect* bounding_rect)
{
    rect->right = min(rect->right, bounding_rect->right);
    rect->bottom = min(rect->bottom, bounding_rect->bottom);
    rect->left = max(rect->left, bounding_rect->left);
    rect->top = max(rect->top, bounding_rect->top);
}

// Can be unstaticed if needed
// Clips a rect of screenblock entries to screenblock boundaries
static void clip_se_rect_to_screenblock(Rect* rect)
{
    clip_se_rect_to_bounding_rect(rect, &FULL_SCREENBLOCK_RECT);
}

// Clips a rect of screenblock entries to be within one step of 
// screenblock boundaries vertically (1 step from top, 1 step from bottom).
static void clip_se_rect_within_step_of_full_screen_vert(Rect* se_rect)
{
    Rect bounding_rect = FULL_SCREENBLOCK_RECT;
    bounding_rect.top += 1;
    bounding_rect.bottom -= -1;
    clip_se_rect_to_bounding_rect(se_rect, &bounding_rect);
}

void main_bg_se_clear_rect(Rect se_rect)
{
    if (se_rect.left > se_rect.right)
        return;
    // Clip to avoid screenblock overflow
    clip_se_rect_to_screenblock(&se_rect);

    for (int y = se_rect.top; y < se_rect.bottom; y++)
    {
        memset16(&(se_mem[MAIN_BG_SBB][se_rect.left + SE_ROW_LEN * y]), 0x0000, rect_width(&se_rect));
    }
}

void main_bg_se_copy_rect_1_tile_vert(Rect se_rect, int direction)
{
    if (se_rect.left > se_rect.right
        || (direction != SE_UP && direction != SE_DOWN))
    {
        return;
    }

    // Clip to avoid read/write overflow of the screenblock
    clip_se_rect_within_step_of_full_screen_vert(&se_rect);

    int start = (direction == SE_UP) ? se_rect.top : se_rect.bottom;
    int end = (direction == SE_UP) ? se_rect.bottom : se_rect.top;

    for (int y = start; y != end - direction; y -= direction)
    {
        memcpy16(&se_mem[MAIN_BG_SBB][se_rect.left + SE_ROW_LEN * (y + direction)],
                 &se_mem[MAIN_BG_SBB][se_rect.left + SE_ROW_LEN * y], 
                 rect_width(&se_rect));
    }
}

void main_bg_se_move_rect_1_tile_vert(Rect se_rect, int direction)
{
    if (se_rect.left > se_rect.right
        || (direction != SE_UP && direction != SE_DOWN))
    {
        return;
    }

    // Clip to avoid read/write overflow of the screenblock
    clip_se_rect_within_step_of_full_screen_vert(&se_rect);

    int deleted_y = (direction == SE_UP) ? se_rect.bottom : se_rect.top;

    main_bg_se_copy_rect_1_tile_vert(se_rect, direction);

    memset16(&se_mem[MAIN_BG_SBB][se_rect.left + SE_ROW_LEN * (deleted_y)], 0x0000, rect_width(&se_rect));
}

void main_bg_se_copy_rect(Rect rect_to, Rect rect_from)
{
    if (rect_from.left > rect_from.right
        || rect_from.top > rect_from.bottom
        || rect_to.left > rect_to.right
        || rect_from.top > rect_from.bottom)
    {
        return;
    }

    clip_se_rect_to_screenblock(&rect_to);
    clip_se_rect_to_screenblock(&rect_from);

    for (int y = 0; y < rect_height(&rect_to); y++)
    {
        memcpy16(&(se_mem[MAIN_BG_SBB][rect_to.left + SE_ROW_LEN * (rect_to.top + y)]),
                 &(se_mem[MAIN_BG_SBB][rect_from.left + SE_ROW_LEN * (rect_from.top + y)]),
                 rect_width(&rect_to));
    }
}

void tte_erase_rect_wrapper(Rect rect)
{
    tte_erase_rect(rect.left, rect.top, rect.right, rect.bottom);
}

