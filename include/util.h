/**
 * @file util.h
 *
 * @brief Helper utilities sizes of numbers
 */
#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

/**
 * @def UNDEFINED
 * @brief Universal representation of an undefined value for ints
 */
#define UNDEFINED -1

/**
 * @def NUM_ELEM_IN_ARR
 *
 * @param arr input array
 */
#define NUM_ELEM_IN_ARR(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * @def INT_MAX_DIGITS
 * @brief Number of characters in the maximum representation of an integer
 *
 * For example: strlen(str(INT_MAX)) = strlen("-2147483647")
 */
#define INT_MAX_DIGITS 11

/**
 * @def UINT_MAX_DIGITS
 * @brief Number of characters in the maximum representation of an unsigned integer
 *
 * For example: strlen(str(UINT32_MAX)) = strlen("4294967295")
 */
#define UINT_MAX_DIGITS 10

/**
 * @def UINT8_MAX_DIGITS
 * @brief Number of characters in the maximum representation of an unsigned char
 *
 * For example: strlen(str(UINT8_MAX)) = strlen("255")
 */
#define UINT8_MAX_DIGITS 3 // strlen(str(UINT8_MAX)) = strlen("255")

/**
 * @brief Avoid uint 32 overflow when adding
 *
 * @param a left operator a + b
 * @param b left operator a + b
 *
 * @return the result of a + b or UINT32_MAX
 */
uint32_t u32_protected_add(uint32_t a, uint32_t b);

/**
 * @brief Avoid uint 16 overflow when adding
 *
 * @param a left operator a + b
 * @param b left operator a + b
 *
 * @return the result of a + b or UINT16_MAX
 */
uint16_t u16_protected_add(uint16_t a, uint16_t b);

/**
 * @brief Avoid uint 32 overflow when multiplying score
 *
 * @param a left operator a x b
 * @param b left operator a x b
 *
 * @return the result of a x b or UINT32_MAX
 */
uint32_t u32_protected_mult(uint32_t a, uint32_t b);

/**
 * @brief Avoid uint 16 overflow when multiplying score
 *
 * @param a left operator a x b
 * @param b left operator a x b
 *
 * @return the result of a x b or UINT16_MAX
 */
uint16_t u16_protected_mult(uint16_t a, uint16_t b);

/**
 * @brief Get the number of digits in a number
 * https://stackoverflow.com/questions/1068849/how-do-i-determine-the-number-of-digits-of-an-integer-in-c
 *
 * @param n value to find the number of decimal digits of
 *
 * @return the number of digits in a number
 */
static inline int get_digits(int n)
{
    if (n < 10)
        return 1;
    if (n < 100)
        return 2;
    if (n < 1000)
        return 3;
    if (n < 10000)
        return 4;
    if (n < 100000)
        return 5;
    if (n < 1000000)
        return 6;
    if (n < 10000000)
        return 7;
    if (n < 100000000)
        return 8;
    if (n < 1000000000)
        return 9;
    return 10;
}

/**
 * @brief Get the number of digits closest to it's "odd" value
 *
 * Useful for centering text on tiles
 *
 * @param n value to find the number of odd decimal digits of
 *
 * @return The closest number of digits that is an odd value.
 */
static inline int get_digits_odd(int n)
{
    if (n < 100)
        return 1;
    if (n < 10000)
        return 2;
    if (n < 1000000)
        return 3;
    if (n < 100000000)
        return 4;
    return 5;
}

/**
 * @brief Get the number of digits closest to it's "even" value
 *
 * Useful for centering text on tiles
 *
 * @param n value to find the number of even decimal digits of
 *
 * @return The closest number of digits that is an even value.
 */
static inline int get_digits_even(int n)
{
    if (n < 10)
        return 1;
    if (n < 1000)
        return 2;
    if (n < 100000)
        return 3;
    if (n < 10000000)
        return 4;
    return 5;
}

#endif // UTIL_H
