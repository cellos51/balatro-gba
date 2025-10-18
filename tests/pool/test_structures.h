#ifndef POOL_TEST_STRUCTURES
#define POOL_TEST_STRUCTURES

#include <stddef.h>

typedef struct 
{
    union 
    {
        int my_int;
        float my_float;
        char my_char;
    } GenericData;
    
    int my_type;
} ChunkOfData;

#endif // POOL_TEST_STRUCTURES
