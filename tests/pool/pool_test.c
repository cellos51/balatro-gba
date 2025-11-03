#include "pool.h"

#include "util.h"
#include "test_structures.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct timespec timestamp_t;

timestamp_t get_time(void)
{
    timestamp_t t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t;
}

void print_time_diff(timestamp_t start, timestamp_t end)
{
    int64_t diff_nsec = end.tv_nsec - start.tv_nsec;

    printf("Elapsed: %ld ns\n", diff_nsec);
}

int get_random(int low, int high)
{
    srand(time(NULL));
    int n = low + rand() % (high - low + 1);
    return n;
}

bool test_fill(ChunkOfData* myPtrs[], int check_size)
{
    int itr = 0;

    ChunkOfData* test_chunk = NULL;
    do
    {
        test_chunk = POOL_GET(ChunkOfData);
        if(test_chunk != NULL) myPtrs[itr++] = test_chunk;
    } while(test_chunk != NULL);

    if(itr != check_size)
    {
        fprintf(stderr, "Error: failed to get expected number of valid pointers\n"
                        "    expected: %d, actual %d\n", check_size, itr);
        return false;
    }

    return true;
}

bool test_fill_and_empty(void)
{
    ChunkOfData* myPtrs[TEST_SIZE];
    if(!test_fill(myPtrs, TEST_SIZE)) return false;

    for(int itr = (TEST_SIZE - 1); itr >= 0; --itr)
    {
        POOL_FREE(ChunkOfData, myPtrs[itr]);
        myPtrs[itr] = NULL;
    }

    return true;
}

bool test_fill_and_remove_at_random_and_refill_and_empty(void)
{
    ChunkOfData* myPtrs[TEST_SIZE];
    if(!test_fill(myPtrs, TEST_SIZE)) return false;

    // remove between 10 and TEST_SIZE 
    int number_to_remove = get_random(100, TEST_SIZE);

    for(int itr = 0; itr < number_to_remove; itr++)
    {
        POOL_FREE(ChunkOfData, myPtrs[itr]);
        myPtrs[itr] = NULL;
    }

    if(!test_fill(myPtrs, number_to_remove)) return false;

    for(int itr = 0; itr < TEST_SIZE; itr++)
    {
        POOL_FREE(ChunkOfData, myPtrs[itr]);
        myPtrs[itr] = NULL;
    }

    return true;
}

int main(void)
{
    // Test it twice to make sure empty works, kinda hacky.
    printf("Testing Pool Fill and Empty 1x.\n");
    if(!test_fill_and_empty()) return UNDEFINED;
    printf("Testing Pool Fill and Empty 2x.\n");
    if(!test_fill_and_empty()) return UNDEFINED;

    // Similarly here, verify that fill, random num removal, refill, and empty
    // by refilling and emptying again
    printf("Testing Pool Fill, Partial Empty, Refill, Empty.\n");
    if(!test_fill_and_remove_at_random_and_refill_and_empty()) return UNDEFINED;

    printf("Testing Pool Fill and Empty.\n");
    if(!test_fill_and_empty()) return UNDEFINED;

    printf("-------------------------------------------------------------------------------\n");
    printf("Pool Tests Passed :)\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("Testing execution time for fun :)\n\n");

    timestamp_t t1 = get_time();
    ChunkOfData* myData = POOL_GET(ChunkOfData); 
    timestamp_t t2 = get_time();
    printf("Pool Get One:\n\t");
    print_time_diff(t1, t2);
    printf("\n");

    t1 = get_time();
    POOL_FREE(ChunkOfData, myData);
    t2 = get_time();
    printf("Pool Free One:\n\t");
    print_time_diff(t1, t2);
    printf("\n");

    t1 = get_time();
    myData = (ChunkOfData*)malloc(sizeof(ChunkOfData));
    t2 = get_time();
    printf("Malloc One:\n\t");
    print_time_diff(t1, t2);
    printf("\n");

    t1 = get_time();
    free(myData);
    t2 = get_time();
    printf("Free One:\n\t");
    print_time_diff(t1, t2);
    printf("\n");

    t1 = get_time();
    if(!test_fill_and_empty()) return UNDEFINED;
    t2 = get_time();
    printf("Fill and Empty Pool %d times:\n\t", TEST_SIZE);
    print_time_diff(t1, t2);
    printf("\n");

    t1 = get_time();
    ChunkOfData* myPtrs[TEST_SIZE];
    for(int i = 0; i < TEST_SIZE; i++) 
    {
        myPtrs[i] = (ChunkOfData*)malloc(sizeof(ChunkOfData));
    }
    for(int i = 0; i < TEST_SIZE; i++) 
    {
        free(myPtrs[i]);
    }
    t2 = get_time();
    printf("Fill and Empty Malloc %d times:\n\t", TEST_SIZE);
    print_time_diff(t1, t2);

    return 0;
}
