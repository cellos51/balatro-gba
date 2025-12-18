/** @file sort.h
 *
 *  @brief Generic sorting algorithms
 */
#ifndef SORT_H
#define SORT_H

#include <stdbool.h>

/**
 * @brief Arguments for sorting algorithms
 */
typedef struct SortArgs
{
    /**
     * @brief Array of pointers to sort
     */
    void** array;

    /**
     * @brief Number of elements in the array
     */
    int size;

    /**
     * @brief Comparison function that returns true if first arg should come before second arg
     */
    bool (*compare)(void*, void*);
} SortArgs;

/**
 * @brief Generic insertion sort implementation
 * 
 * @param args Arguments for the sorting algorithm
 */
void insertion_sort(SortArgs args);

#endif // SORT_H
