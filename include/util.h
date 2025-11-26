#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define GBLA_UNUSED __attribute__((unused))

static inline int get_digits(int n) // https://stackoverflow.com/questions/1068849/how-do-i-determine-the-number-of-digits-of-an-integer-in-c
{
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    return 10;
}

static inline int get_digits_odd(int n)
{
    if (n < 100) return 1;
    if (n < 10000) return 2;
    if (n < 1000000) return 3;
    if (n < 100000000) return 4;
    return 5;
}

static inline int get_digits_even(int n)
{
    if (n < 10) return 1;
    if (n < 1000) return 2;
    if (n < 100000) return 3;
    if (n < 10000000) return 4;
    return 5;
}

#define UNDEFINED -1

#define NUM_ELEM_IN_ARR(arr) (sizeof(arr) / sizeof((arr)[0]))

#define INT_MAX_DIGITS  11 // strlen(str(INT_MAX)) = strlen("-2147483647")
#define UINT_MAX_DIGITS 10 // strlen(str(UINT32_MAX)) = strlen("4294967295")
#define UINT8_MAX_DIGITS 3 // strlen(str(UINT8_MAX)) = strlen("255")

#define ONE_K 1000
#define ONE_M 1000000
#define ONE_B 1000000000

#define ONE_K_ZEROS 3
#define ONE_M_ZEROS 6
#define ONE_B_ZEROS 9

// The suffix replaces everything past the third digit, e.g. "999K" -> "1M"
// so it needs at least this number of chars to be able to display any suffixed number
#define SUFFIXED_NUM_MIN_REQ_CHARS 4

int int_arr_max(int int_arr[], int size);

// TODO: Documentation
void truncate_uint_to_suffixed_str(uint32_t num, int num_places, char out_str[UINT_MAX_DIGITS + 2]);

uint32_t u32_protected_add (uint32_t a, uint32_t b);
uint16_t u16_protected_add (uint16_t a, uint16_t b);
uint32_t u32_protected_mult(uint32_t a, uint32_t b);
uint16_t u16_protected_mult(uint16_t a, uint16_t b);

#endif // UTIL_H
