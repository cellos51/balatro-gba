#ifndef SORT_H
#define SORT_H

#include <stdbool.h>

/**
 * Generic insertion sort implementation
 * 
 * @param array Array of pointers to sort
 * @param size Number of elements in the array
 * @param compare Comparison function that returns true if first arg should come before second arg
 */
void insertion_sort(void** array, int size, bool (*compare)(void*, void*));

#endif // SORT_H
