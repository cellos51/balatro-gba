#include "sort.h"

void insertion_sort(void** array, int size, bool (*compare)(void*, void*))
{
    for (int i = 1; i < size; i++)
    {
        void* key = array[i];
        int j;

        // Shift elements that don't satisfy the comparison
        for (j = i - 1; j >= 0 && !compare(array[j], key); j--)
        {
            array[j + 1] = array[j];
        }
        array[j + 1] = key;
    }
}
