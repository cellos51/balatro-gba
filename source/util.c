#include "util.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

int int_arr_max(int int_arr[], int size)
{
    int max = INT_MIN;
    for (int i = 0; i < size; i++)
    {
        if (int_arr[i] > max)
        {
            max = int_arr[i];
        }
    }

    return max;
}

void truncate_uint_to_suffixed_str(
    uint32_t num,
    int num_req_chars,
    char out_str_buff[UINT_MAX_DIGITS + 1]
)
{
    bool inevitable_overflow = num_req_chars < SUFFIXED_NUM_MIN_REQ_CHARS;
    if (inevitable_overflow)
    {
        num_req_chars = SUFFIXED_NUM_MIN_REQ_CHARS;
    }

    int num_digits = u32_get_digits(num);
    int overflow_size = num_digits - num_req_chars;
    char* suffix = "";

    /* If there is overflow, divide by the next suffixed power of 10
     * to truncate the number back within num_req_chars.
     * UINT32_MAX is in the billions so no need to check larger numbers
     * or perform complex mathematical operations.
     */
    if (overflow_size >= ONE_M_ZEROS)
    {
        num /= ONE_B;
        suffix = "B";
    }
    else if (overflow_size >= ONE_K_ZEROS)
    {
        num /= ONE_M;
        suffix = "M";
    }
    else if (overflow_size > 0 || (inevitable_overflow && num_digits == SUFFIXED_NUM_MIN_REQ_CHARS))
    // Special case - alleviate inevitable overflow for 1000s and truncate them to "1K"s
    {
        num /= ONE_K;
        suffix = "K";
    }

    snprintf(out_str_buff, UINT_MAX_DIGITS + 1, "%lu%s", num, suffix);
}

// Avoid uint overflow when add/multiplying score

uint32_t u32_protected_add(uint32_t a, uint32_t b)
{
    return (a > (UINT32_MAX - b)) ? UINT32_MAX : (a + b);
}

uint16_t u16_protected_add(uint16_t a, uint16_t b)
{
    return (a > (UINT16_MAX - b)) ? UINT16_MAX : (a + b);
}

uint32_t u32_protected_mult(uint32_t a, uint32_t b)
{
    return (a == 0 || b == 0) ? 0 : (a > (UINT32_MAX / b) ? UINT32_MAX : a * b);
}

uint16_t u16_protected_mult(uint16_t a, uint16_t b)
{
    return (a == 0 || b == 0) ? 0 : (a > (UINT16_MAX / b) ? UINT16_MAX : a * b);
}
