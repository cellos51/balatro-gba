#include "util.h"
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>

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

void truncate_uint_to_suffixed_str(uint32_t num, int num_places, char out_str_buff[UINT_MAX_DIGITS + 2])
{
    bool inevitable_overflow = num_places < SUFFIXED_NUM_MIN_PLACES;
    if (inevitable_overflow)
    {
        num_places = SUFFIXED_NUM_MIN_PLACES;
    }

    int num_digits = get_digits(num);
    int overflow_size = num_digits - num_places;
    char* suffix = "";
    
    /* UINT32_MAX is in the billions so no need to check larger numbers
     * or perform complext mathmetaical operations.
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
    else if (overflow_size > 0 
            || (inevitable_overflow && num_digits == 4)) 
            // Special case - alleviate inevitable overflow for "1000" and truncate it "1K"
    {
        num /= ONE_K;
        suffix = "K";
    }

    snprintf(out_str_buff, UINT_MAX_DIGITS + 2, "%lu%s", num, suffix);
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
