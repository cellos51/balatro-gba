#include "util.h"

#include "font.h"

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
    uint32_t truncated_num = num;
    int num_digits = u32_get_digits(num);
    uint32_t remainder = 0; // The decimal point remainder
    bool overflow = num_digits > num_req_chars;
    char* suffix = "";
    char remainder_str[INT_MAX_DIGITS + 1];
    remainder_str[0] = '\0';

    /* If there is overflow, divide by the next suffixed power of 10
     * to truncate the number back within num_req_chars.
     * UINT32_MAX is in the billions so no need to check larger numbers
     * or perform complex mathematical operations.
     */
    if (num >= ONE_B && overflow)
    {
        // TODO: Optimize division and remainder
        remainder = num % ONE_B;
        truncated_num /= ONE_B;
        suffix = "B";
    }
    else if (num >= ONE_M && overflow)
    {
        remainder = num % ONE_M;
        truncated_num /= ONE_M;
        suffix = "M";
    }
    else if (num >= ONE_K && overflow)
    {
        remainder = num % ONE_K;
        truncated_num /= ONE_K;
        suffix = "K";
    }

    if (suffix[0] != '\0' && remainder != 0)
    {
        // TODO: Extract to function
        snprintf(remainder_str, sizeof(remainder_str), "%lu", remainder);

        int remaining_chars = num_req_chars - u32_get_digits(truncated_num) - 1; // - 1 for suffix

        // Truncate overflow
        remainder_str[remaining_chars] = '\0';

        int truncated_remainder_digits = remaining_chars;

        while (truncated_remainder_digits > 0 &&
               remainder_str[truncated_remainder_digits - 1] == '0')
        {
            truncated_remainder_digits--;
        }

        // Truncate 0s
        remainder_str[truncated_remainder_digits] = '\0';

        if (remainder_str[0] != '\0')
        {
            // TODO: Fix hacky code
            remainder_str[0] = get_font_point_str(remainder_str[0] - '0')[0];
        }
    }

    // TODO: fix snprintf buffer size...
    snprintf(
        out_str_buff,
        2 * UINT_MAX_DIGITS + 1,
        "%lu%s%s",
        truncated_num,
        remainder_str,
        suffix
    );
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
