#include "util.h"
#include <limits.h>

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
