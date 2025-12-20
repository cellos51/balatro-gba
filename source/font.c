#include "font.h"

#include "util.h"

#include <stdlib.h>

static const char* s_font_point_lookup[] = {
    XSTR(FP0_CHAR),
    XSTR(FP1_CHAR),
    XSTR(FP2_CHAR),
    XSTR(FP3_CHAR),
    XSTR(FP4_CHAR),
    XSTR(FP5_CHAR),
    XSTR(FP6_CHAR),
    XSTR(FP7_CHAR),
    XSTR(FP8_CHAR),
    XSTR(FP9_CHAR),
};

const char* get_font_point_str(int val)
{
    val = abs(val) % 10;
    return s_font_point_lookup[val];
}

char digit_char_to_font_point(char digit_char)
{
    return get_font_point_str(digit_char - '0')[0];
}
