/**
 * @file font.h
 *
 * @brief Utility file to interact with custom font
 */
#ifndef FONT_H
#define FONT_H

#include <tonc.h>

extern const TFont gbalatro_sys8Font;
extern const unsigned int gbalatro_sys8Glyphs[192];

/** @name Decimal Point Fonts
 *  @brief A set of macros to map the fonts decimal-point values (e.g. ".1")
 *  to their replaced characters.
 *
 *  "FP#" -> Font "Point" "number"
 *
 *  This is used at compile time directly with the help of libtonc's XSTR() macro
 *  **Note**: This should **ONLY** be used with the XSTR() macro, otherwise it
 *  is undefined behavior.
 *
 *  For example:
 *  ```c
 *  tte_printf("Testing " XSTR(FP0_CHAR) " Something!");
 *  ```
 *  prints "Testing .0 Something!"
 *
 *  The map is the following:
 *
 *  ```c
 *  '&' == '.0'
 *  '^' == '.1'
 *  '{' == '.2'
 *  '}' == '.3'
 *  '|' == '.4'
 *  '`' == '.5'
 *  '<' == '.6'
 *  '>' == '.7'
 *  '_' == '.8'
 *  ';' == '.9'
 *  ```
 *
 * @{
 */
#define FP0_CHAR & // .0
#define FP1_CHAR ^ // .1
#define FP2_CHAR { // .2
#define FP3_CHAR } // .3
#define FP4_CHAR | // .4
#define FP5_CHAR ` // .5
#define FP6_CHAR < // .6
#define FP7_CHAR > // .7
#define FP8_CHAR _ // .8
#define FP9_CHAR ; // .9
/** @} */

/**
 * @brief Get the decimal point string equivalent of 0-9 from integer param.
 *
 * @param val Value between 0-9 to get ".0" to ".9" equivalents. Note that
 *            if a value outside of range is passed it will perform '% 10'
 *            on that value and the last decimal digit will be used.
 *
 * @return A char* to the string values of '.0' to '.9'
 */
const char* get_font_point_str(int val);

#endif // FONT_H
