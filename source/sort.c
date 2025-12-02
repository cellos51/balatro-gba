#include "sort.h"

void insertion_sort(SortArgs args)
{
    for (int i = 1; i < args.size; i++)
    {
        void* key = args.array[i];
        int j;

        // Shift elements that don't satisfy the comparison
        for (j = i - 1; j >= 0 && !args.compare(args.array[j], key); j--)
        {
            args.array[j + 1] = args.array[j];
        }
        args.array[j + 1] = key;
    }
}
