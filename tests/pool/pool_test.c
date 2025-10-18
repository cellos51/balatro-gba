#include "pool.h"
#include "test_structures.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

    for(int itr = TEST_SIZE; itr >= 0; --itr)
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
    if(!test_fill_and_empty()) return -1;
    if(!test_fill_and_empty()) return -1;

    // Similarly here, verify that fill, random num removal, refill, and empty
    // by refilling and emptying again
    if(!test_fill_and_remove_at_random_and_refill_and_empty()) return -1;
    if(!test_fill_and_empty()) return -1;

    printf("Pool Tests Passed\n");

    return 0;
}
