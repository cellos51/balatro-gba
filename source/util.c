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

#define UINT32_MAX 4294967295
#define UINT16_MAX 65535

unsigned long u32_protected_add(unsigned long a, unsigned long b)
{
    return (a > (UINT32_MAX - b)) ? UINT32_MAX : (a + b);
}
    
unsigned int u16_protected_add(unsigned int a, unsigned int b)
{
    return (a > (UINT16_MAX - b)) ? UINT16_MAX : (a + b);
}

unsigned long u32_protected_mult(unsigned long a, unsigned long b)
{
    return (a == 0 || b == 0) ? 0 : (a > (UINT32_MAX / b) ? UINT32_MAX : a * b);
}

unsigned int u16_protected_mult(unsigned int a, unsigned int b)
{
    return (a == 0 || b == 0) ? 0 : (a > (UINT16_MAX / b) ? UINT16_MAX : a * b);
}