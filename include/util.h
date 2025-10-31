#ifndef UTIL_H
#define UTIL_H

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

#define INT_MAX_DIGITS 10 // strlen(str(INT_MAX)) = strlen("2147483647")

int int_arr_max(int int_arr[], int size);

#endif // UTIL_H