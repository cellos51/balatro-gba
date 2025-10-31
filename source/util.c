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
