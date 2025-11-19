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
u32 u32_protected_add(u32 a, u32 b)
{
    return (a > (UINT32_MAX - b)) ? UINT32_MAX : (a + b);
}
    
u16 u16_protected_add(u16 a, u16 b)
{
    return (a > (UINT16_MAX - b)) ? UINT16_MAX : (a + b);
}

u32 u32_protected_mult(u32 a, u32 b)
{
    return (a == 0 || b == 0) ? 0 : (a > (UINT32_MAX / b) ? UINT32_MAX : a * b);
}

u16 u16_protected_mult(u16 a, u16 b)
{
    return (a == 0 || b == 0) ? 0 : (a > (UINT16_MAX / b) ? UINT16_MAX : a * b);
}