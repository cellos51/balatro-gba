#include "font.h"
#include <stdlib.h>

static const char* s_font_point_lookup[] = {
    __XSTRING(FP0_CHAR),
    __XSTRING(FP1_CHAR),
    __XSTRING(FP2_CHAR),
    __XSTRING(FP3_CHAR),
    __XSTRING(FP4_CHAR),
    __XSTRING(FP5_CHAR),
    __XSTRING(FP6_CHAR),
    __XSTRING(FP7_CHAR),
    __XSTRING(FP8_CHAR),
    __XSTRING(FP9_CHAR),
};

const char* get_font_point_str(int val)
{
    val = abs(val) % 10;
    return s_font_point_lookup[val];
}
