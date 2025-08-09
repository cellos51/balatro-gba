#ifndef AFFINE_BACKGROUND_H
#define AFFINE_BACKGROUND_H

#include <tonc.h>

#include "graphic_utils.h"

#define AFFINE_BG_PAL_LEN 5
#define AFFINE_BG_PB (PAL_ROW_LEN * 15) // This isn't really a palette bank, just the starting index of the palette

void affine_background_init();
void affine_background_hblank();
void affine_background_update();
void affine_background_set_color(COLOR color);
// Must be called with an array of size at least  AFFINE_BG_PAL_LEN
void affine_background_load_palette(const u16 *src);

#endif // AFFINE_BACKGROUND_H
