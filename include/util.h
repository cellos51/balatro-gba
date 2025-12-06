/**
 * @file util.h
 *
 * @brief Utilities relating around number string representation and protected arithmatic helper functions
 */
#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

/**
 * @def GBLA_UNUSED
 * @brief A friendly wrapper around the not so friendly looking __attribute__ syntax for ((unused))
 */
#define GBLA_UNUSED __attribute__((unused))

/**
 * @def UNDEFINED
 * @brief Universal representation of an undefined value
 */
#define UNDEFINED -1

/**
 * @def NUM_ELEM_IN_ARR
 * @brief Get the number of elements in an array
 *
 * @param arr input array
 */
#define NUM_ELEM_IN_ARR(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * @def INT_MAX_DIGITS
 * @brief Number of characters in the maximum representation of an integer
 *
 * **For example: strlen(str(INT_MAX)) = strlen("-2147483647")**
 */
#define INT_MAX_DIGITS 11

/**
 * @def UINT_MAX_DIGITS
 * @brief Number of characters in the maximum representation of an unsigned integer
 *
 * **For example: strlen(str(UINT32_MAX)) = strlen("4294967295")**
 */
#define UINT_MAX_DIGITS 10

/**
 * @def UINT8_MAX_DIGITS
 * @brief Number of characters in the maximum representation of an unsigned char
 *
 * **For example: strlen(str(UINT8_MAX)) = strlen("255")**
 */
#define UINT8_MAX_DIGITS 3

/**
 * @brief Avoid overflow when adding two u32 integers
 *
 * @param a left operator **a + b**
 * @param b left operator **a + b**
 *
 * @return the result of **a + b** or **UINT32_MAX** in case of overflow
 */
uint32_t u32_protected_add(uint32_t a, uint32_t b);

/**
 * @brief Avoid overflow when adding two u16 integers
 *
 * @param a left operator **a + b**
 * @param b left operator **a + b**
 *
 * @return the result of **a + b** or **UINT16_MAX** in case of overflow
 */
uint16_t u16_protected_add(uint16_t a, uint16_t b);

/**
 * @brief Avoid overflow when multiplying two u32 integers
 *
 * @param a left operator **a * b**
 * @param b left operator **a * b**
 *
 * @return the result of **a * b** or **UINT32_MAX** in case of overflow
 */
uint32_t u32_protected_mult(uint32_t a, uint32_t b);

/**
 * @brief Avoid overflow when multiplying two u16 integers
 *
 * @param a left operator **a * b**
 * @param b left operator **a * b**
 *
 * @return the result of **a * b** or **UINT16_MAX** in case of overflow
 */
uint16_t u16_protected_mult(uint16_t a, uint16_t b);

/**
 * @brief Truncate a uint to a suffixed stream
 *
 * @param num number to convert
 * @param num_req_chars Number of chars to display
 * #param out_str_buff buffer that string will be saved too
 */
void truncate_uint_to_suffixed_str(uint32_t num, int num_req_chars, char out_str_buff[UINT_MAX_DIGITS + 1]);

/**
 * @brief Get the number of digits in a number
 * https://stackoverflow.com/questions/1068849/how-do-i-determine-the-number-of-digits-of-an-integer-in-c
 *
 * @param n value to find the number of decimal digits of
 *
 * @return the number of digits in a number
 */
static inline int u32_get_digits(uint32_t n) // https://stackoverflow.com/questions/1068849/how-do-i-determine-the-number-of-digits-of-an-integer-in-c
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
 * @brief Get the number of digits closest to it's "even" value, i.e. ceil(digits/2)
 *
 * Useful for centering text on tiles
 *
 * @param n value to find the number of even decimal digits of
 *
 * @return The value of ceil(get_digits(n)/2)
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
